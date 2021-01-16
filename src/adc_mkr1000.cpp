#include "adc.h"
#include "constants.h"
#include "debug.h"
#include "dma.h"



#ifdef MKR1000



const uint8_t ENABLE = 1;
const uint8_t DISABLE = 0;

void setupSamplePin()
{
    // Input pin for ADC Arduino A3/PA04
    REG_PORT_DIRCLR1 = PORT_PA04;

    // Enable multiplexing on PA04
    PORT->Group[0].PINCFG[4].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[1].reg = PORT_PMUX_PMUXE_B | PORT_PMUX_PMUXO_B;
}

void mADC::init()
{
    logTrace("ADC: init");
    setupSamplePin();

    // Ensure ADC is disabled to configure it
    ADC->CTRLA.bit.ENABLE = DISABLE;
    // Wait until ADC is DISABLED
    while (ADC->STATUS.bit.SYNCBUSY == 1);


    // Increased resolution is not needed in 8 bit mode i guess
    ADC->REFCTRL.bit.REFCOMP = DISABLE;
    // Set Vref to 1/2 Vin (usually 3.3V -> Vref = 1.65V)
    ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;


    // Set gain according to REFCTRL?
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
    ADC->INPUTCTRL.bit.INPUTOFFSET = DISABLE;
    // Set measuring between GND and PIN4
    // MUXNEG should be unused since CTRLB.DIFFMODE is DISABLED
    ADC->INPUTCTRL.bit.MUXNEG = ADC_INPUTCTRL_MUXNEG_GND_Val;
    ADC->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_PIN4_Val;


    // Disable averaging since we are only interested in a simple sample
    ADC->AVGCTRL.bit.ADJRES = 0;
    ADC->AVGCTRL.bit.SAMPLENUM = 1;


    // Not extra half sample cycles per sample
    ADC->SAMPCTRL.bit.SAMPLEN = 0;


    ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV16_Val;
    // Set results to 8bit resolution
    ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_8BIT_Val;
    ADC->CTRLB.bit.FREERUN = ENABLE;


    // Start ADC
    ADC->CTRLA.bit.ENABLE = ENABLE;
    // Wait until ADC is ENABLED
    while (ADC->STATUS.bit.SYNCBUSY == 1);
}

uint32_t triggerValue = 0;
bool triggerActive = false;

void mADC::setTrigger(uint8_t _channel, bool _active, uint32_t _mV)
{
    if(_channel != 1)
    {
        logWarning("Any channels besides 1 are not supported by this board");
        return;
    }
    if(!_active)
    {
        mDMA::suspend();
        return;
    }
    const uint32_t maxSampleValue = (1<<BOARD_RESOLUTION)-1;
    triggerValue = (maxSampleValue * _mV) / BOARD_V_REF;
    triggerActive = _active;

    mDMA::resume();
}

uint8_t* lastBuffer = nullptr;

void mADC::handleEvents()
{
    uint8_t* buffer = mDMA::getLastCompletedBuffer();
    if(buffer == nullptr || buffer == lastBuffer || !triggerActive)
    {
        return;
    }
    lastBuffer = buffer;

    logTrace("Checking buffer");
    if(buffer[0] < triggerValue)
    {
        for(uint32_t i=0;i<ADC_BUFFER_SIZE;i++)
            if(buffer[i] >= triggerValue)
            {
                mDMA::triggerHit();
                triggerActive = false;
                return;
            }
    }
    else
    {
        for(uint32_t i=0;i<ADC_BUFFER_SIZE;i++)
            if(buffer[i] <= triggerValue)
            {
                mDMA::triggerHit();
                triggerActive = false;
                return;
            }
    }
    
}

#endif  // MKR1000
