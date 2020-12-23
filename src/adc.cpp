#include "adc.h"
#include "constants.h"
#include "debug.h"


typedef struct {
    uint16_t btctrl;
    uint16_t btcnt;
    uint32_t srcaddr;
    uint32_t dstaddr;
    uint32_t descaddr;
} dmacdescriptor ;
volatile dmacdescriptor wrb[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_section[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptors[2] __attribute__ ((aligned (16)));

volatile uint32_t dmadone;
volatile uint32_t completed_buffer;
uint8_t b_1[ADC_BUFFER_SIZE],
        b_2[ADC_BUFFER_SIZE];
uint8_t* buffers[] = {b_1, b_2};
volatile bool _triggerHit = false;
sendBufferCallback _sendBufferCallback;

static __inline__ void ADCsync() __attribute__((always_inline, unused));
static void   ADCsync() {
  while (ADC->STATUS.bit.SYNCBUSY == 1); //Just wait till the ADC is free
}

//setup measurement pin, using Arduino ADC pin A3
void setAdcPin() {
  // Input pin for ADC Arduino A3/PA04
  REG_PORT_DIRCLR1 = PORT_PA04;

  // Enable multiplexing on PA04
  PORT->Group[0].PINCFG[4].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[1].reg = PORT_PMUX_PMUXE_B | PORT_PMUX_PMUXO_B;
}

// ----- INIT -----
void initDma() {
    // probably on by default
    PM->AHBMASK.reg |= PM_AHBMASK_DMAC ;
    PM->APBBMASK.reg |= PM_APBBMASK_DMAC ;
    NVIC_EnableIRQ( DMAC_IRQn ) ;

    DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
    DMAC->WRBADDR.reg = (uint32_t)wrb;
    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);
}

void initAdc(){
  setAdcPin();

  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  ADCsync();
  
  // Select reference
  REG_ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTVCC1; //set vref for ADC to VCC
  ADCsync();
  
  ADC->INPUTCTRL.reg |= ADC_INPUTCTRL_GAIN_DIV2 | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN4;
  ADCsync();
  
  // Average control 1 sample, no right-shift
  REG_ADC_AVGCTRL |= ADC_AVGCTRL_SAMPLENUM_1;
  
  // Sampling time, no extra sampling half clock-cycles12
  REG_ADC_SAMPCTRL |= ADC_SAMPCTRL_SAMPLEN(0);
  ADCsync();
  
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV16 | ADC_CTRLB_RESSEL_8BIT | ADC_CTRLB_FREERUN;
  ADCsync();
  
  ADC->CTRLA.bit.ENABLE = 0x01;
  ADCsync();
}

// ----- INTERUPTS -----
void DMAC_Handler() {
    // interrupts DMAC_CHINTENCLR_TERR DMAC_CHINTENCLR_TCMPL DMAC_CHINTENCLR_SUSP
    uint8_t active_channel;

    __disable_irq();
    active_channel =  DMAC->INTPEND.reg & DMAC_INTPEND_ID_Msk; // get channel number
    DMAC->CHID.reg = DMAC_CHID_ID(active_channel);
    dmadone = DMAC->CHINTFLAG.reg;
    DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL; // clear
    DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
    DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
    completed_buffer = (completed_buffer + 1) % 2;

    memcpy(&descriptor_section[DMA_CHANNEL],&descriptors[(completed_buffer + 1) % 2], sizeof(dmacdescriptor));
    uint32_t temp_CHCTRLB_reg;
    
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
    DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
    DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << DMA_CHANNEL));
    temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) |
      DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY) | DMAC_CHCTRLB_TRIGACT_BEAT;
    DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
    DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK ; // enable all 3 interrupts
    // start channel
    if(!_triggerHit)
    {
        DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
        DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
    }
    
    __enable_irq();
}

void startAdcSampling() {
    uint32_t temp_CHCTRLB_reg;
    
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
    DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
    DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << DMA_CHANNEL));
    temp_CHCTRLB_reg = DMAC_CHCTRLB_LVL(0) |
      DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY) | DMAC_CHCTRLB_TRIGACT_BEAT;
    DMAC->CHCTRLB.reg = temp_CHCTRLB_reg;
    DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK ; // enable all 3 interrupts
    dmadone = 0;

    descriptors[0].descaddr = 0;//(uint32_t) &descriptors[1];
    descriptors[0].srcaddr = (uint32_t) &ADC->RESULT.reg;
    descriptors[0].btcnt =  ADC_BUFFER_SIZE;
    descriptors[0].dstaddr = (uint32_t)buffers[0] + ADC_BUFFER_SIZE;  // end address
    descriptors[0].btctrl =  DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;
    memcpy(&descriptor_section[DMA_CHANNEL],&descriptors[0], sizeof(dmacdescriptor));

    descriptors[1].descaddr = 0;//(uint32_t) &descriptors[0];
    descriptors[1].srcaddr = (uint32_t) &ADC->RESULT.reg;
    descriptors[1].btcnt =  ADC_BUFFER_SIZE;
    descriptors[1].dstaddr = (uint32_t)buffers[1] + ADC_BUFFER_SIZE;   // end address
    descriptors[1].btctrl =  DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;

    // start channel
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}

void Adc_registerSendBufferCallback(sendBufferCallback _cb)
{
    _sendBufferCallback = _cb;
}

bool _isRunning = false;

void stopSampling()
{
    _isRunning = false;
    logInfo("Sampling stopped");
}

void startSampling()
{
    _isRunning = true;
    logInfo("Sampling stated");
}

void triggerHit(const uint8_t* bufferToCheck)
{
    logInfo("Trigger hit!");
    _triggerHit = true;
    stopSampling();

    _sendBufferCallback(bufferToCheck, ADC_BUFFER_SIZE);

    _triggerHit = false;
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}

void Adc_init()
{
    logTrace("initAdc");
    initAdc();
    logTrace("initDma");
    initDma();
    logTrace("startAdcSampling");
    startAdcSampling();
    logDebug("ADC initialized");
}

uint32_t _triggerValue = 0;

void Adc_setTrigger(uint8_t _channel, bool _active, uint32_t _mV)
{
    if(_channel != 1)
    {
        logWarning("Any channels besides 1 are not supported by this board");
        return;
    }
    if(!_active)
    {
        stopSampling();
        return;
    }
    const uint32_t maxSampleValue = (1<<BOARD_RESOLUTION)-1;
    _triggerValue = (maxSampleValue * _mV) / BOARD_V_REF;
    startSampling();
}

void Adc_handleEvents()
{
    if(!dmadone || !_isRunning)
        return;
    dmadone = 0;
    const uint8_t* bufferToCheck = buffers[completed_buffer];
    if(bufferToCheck[0] < _triggerValue)
    {
        for(uint32_t i=0;i<ADC_BUFFER_SIZE;i++)
            if(bufferToCheck[i] >= _triggerValue)
            {
                triggerHit(bufferToCheck);
                return;
            }
    }
    else
    {
        for(uint32_t i=0;i<ADC_BUFFER_SIZE;i++)
            if(bufferToCheck[i] <= _triggerValue)
            {
                triggerHit(bufferToCheck);
                return;
            }
    }
    
}