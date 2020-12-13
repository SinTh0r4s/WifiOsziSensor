const uint32_t ADCPIN = A3;
const uint32_t DMA_CHANNEL = 0u;


typedef struct {
    uint16_t btctrl;
    uint16_t btcnt;
    uint32_t srcaddr;
    uint32_t dstaddr;
    uint32_t descaddr;
} dmacdescriptor ;
volatile dmacdescriptor wrb[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_section[12] __attribute__ ((aligned (16)));
dmacdescriptor descriptors[NUM_BUFFERS] __attribute__ ((aligned (16)));

volatile uint32_t dmadone;
volatile int32_t completed_buffer;


// ----- HELPER -----
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
    completed_buffer = (completed_buffer + 1) % NUM_BUFFERS;

    memcpy(&descriptor_section[DMA_CHANNEL],&descriptors[(completed_buffer + 1) % NUM_BUFFERS], sizeof(dmacdescriptor));
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
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
    
    __enable_irq();
}

// ----- MAIN -----
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
    descriptors[0].btcnt =  BUFFER_SIZE;
    descriptors[0].dstaddr = (uint32_t)buffers[0] + BUFFER_SIZE;  // end address
    descriptors[0].btctrl =  DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;
    memcpy(&descriptor_section[DMA_CHANNEL],&descriptors[0], sizeof(dmacdescriptor));
    for(uint32_t i=1;i<NUM_BUFFERS-1;i++){
    descriptors[i].descaddr = 0;//(uint32_t) &descriptors[i+1];
    descriptors[i].srcaddr = (uint32_t) &ADC->RESULT.reg;
    descriptors[i].btcnt =  BUFFER_SIZE;
    descriptors[i].dstaddr = (uint32_t)buffers[i] + BUFFER_SIZE;   // end address
    descriptors[i].btctrl =  DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;
    }
    descriptors[NUM_BUFFERS-1].descaddr = 0;//(uint32_t) &descriptors[0];
    descriptors[NUM_BUFFERS-1].srcaddr = (uint32_t) &ADC->RESULT.reg;
    descriptors[NUM_BUFFERS-1].btcnt =  BUFFER_SIZE;
    descriptors[NUM_BUFFERS-1].dstaddr = (uint32_t)buffers[NUM_BUFFERS-1] + BUFFER_SIZE;   // end address
    descriptors[NUM_BUFFERS-1].btctrl =  DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;

    // start channel
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHANNEL);
    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}

void sampleData(){
  bool skip = false; 
  int32_t transmitOnDescriptor = -1;
  completed_buffer = -1;
  
#ifdef DEBUG_SERIAL
  uint32_t t = micros();
#endif

  // fill DMA descriptors and start DMA
  startAdcSampling();

#ifdef DEBUG_SERIAL
    while(!dmadone);  // await DMA done with one buffer
    t = micros() - t;
    Serial.print("10k samples took ");
    Serial.print(t);
    Serial.println("usec");
#endif
  
  digitalWrite(6, HIGH);  // Show ready status
  while (true){
    while(!dmadone);  // await DMA done with one buffer
    
    if(transmitOnDescriptor == completed_buffer){break;}

#ifdef DEBUG_SERIAL
    Serial.print("dmadone : ");
    Serial.println(dmadone);
    Serial.print("completed_buffer : ");
    Serial.println(completed_buffer);
#endif
    
    const uint8_t* buffer = buffers[completed_buffer];
    dmadone = 0;
#ifdef DEBUG_SERIAL
    Serial.print("Any value != 0 is success : ");
    Serial.println(buffer[2]);
#endif
    for(int32_t i=0;i<BUFFER_SIZE;i++){
      if(buffer[i] < TRIGGER_THRESHHOLD * 256){
#ifdef DEBUG_SERIAL
        Serial.println("Trigger hit!");
#endif
        transmitOnDescriptor = (i-1) % NUM_BUFFERS;
        descriptors[transmitOnDescriptor].descaddr = 0;
        digitalWrite(6, LOW);

        //send UDP out
#ifdef DEBUG_SERIAL
        Serial.println("Commencing transmission!");
#endif
        const uint32_t packetSize = 1000;
        for(uint32_t i = 0;i<BUFFER_SIZE/packetSize;i++){
          sendUdp(&buffer[i*packetSize], packetSize);
        }
        if(BUFFER_SIZE % packetSize > 0){
          sendUdp(&buffer[BUFFER_SIZE/packetSize * packetSize], BUFFER_SIZE % packetSize);
        }
        return;
      }
    }
  }
}
