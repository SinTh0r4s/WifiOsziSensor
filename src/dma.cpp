// libs
#include <algorithm>

// local includes
#include "dma.h"
#include "constants.h"
#include "debug.h"
#include "network.h"


const uint8_t ENABLE = 1;
const uint8_t DISABLE = 0;


uint8_t b_0[ADC_BUFFER_SIZE],
        b_1[ADC_BUFFER_SIZE],
        b_2[ADC_BUFFER_SIZE];
uint8_t* buffers[] = {b_0, b_1, b_2};


volatile DmacDescriptor descriptor_writeback_section[12] __attribute__ ((aligned (16)));
DmacDescriptor descriptor_section[12] __attribute__ ((aligned (16)));
DmacDescriptor adc_descriptors[ADC_NUM_BUFFERS] __attribute__ ((aligned (16)));

bool dataWasSent = true;


void swap(uint8_t* buffer1, uint8_t* buffer2, uint32_t length)
{
    for(uint32_t i=0;i<length;i++)
    {
        const uint8_t tmp = buffer1[i];
        buffer1[i] = buffer2[2];
        buffer2[2] = tmp;
    }
}

void initChannel()
{
    // Activate channel (required for channel specific registers)
    DMAC->CHID.bit.ID = DMA_CHANNEL;


    // Ensure the channel is disabled
    DMAC->CHCTRLA.bit.ENABLE = DISABLE;
    // Reselt channel related registers to default
    DMAC->CHCTRLA.bit.SWRST = ENABLE;
    // Wait until reset is complete
    while(DMAC->CHCTRLA.bit.SWRST);


    // Instruct DMA to perform a single beat on each trigger
    DMAC->CHCTRLB.bit.TRIGACT = DMAC_CHCTRLB_TRIGACT_BEAT_Val;
    // Source of trigger is ADC
    DMAC->CHCTRLB.bit.TRIGSRC = ADC_DMAC_ID_RESRDY;
    // Set max priority
    DMAC->CHCTRLB.bit.LVL = 0;


    // First trigger must be placed manually (TODO: Assumption)
    DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << DMA_CHANNEL));


    // Enable all interrupts
    DMAC->CHINTENSET.bit.SUSP = ENABLE;
    DMAC->CHINTENSET.bit.TCMPL = ENABLE;
    DMAC->CHINTENSET.bit.TERR = ENABLE;
}

void initDescriptors()
{
    // Initialize writeback descriptors with 0 to ensure not funny business happens before the first descriptor is complete
    memset((void*)&descriptor_writeback_section[0], 0, sizeof(DmacDescriptor) * 12);

    // Setup next descriptor
    adc_descriptors[0].DESCADDR.bit.DESCADDR = (uint32_t) &adc_descriptors[1];
    // Take bytes from ADC
    adc_descriptors[0].SRCADDR.bit.SRCADDR = (uint32_t) &ADC->RESULT.reg;
    // How many beats to execute
    adc_descriptors[0].BTCNT.bit.BTCNT =  ADC_BUFFER_SIZE;
    // Store bytes to buffer
    adc_descriptors[0].DSTADDR.bit.DSTADDR = (uint32_t)buffers[0] + ADC_BUFFER_SIZE;
    // Instruct DMA to increment destination address after each write to memory
    adc_descriptors[0].BTCTRL.bit.DSTINC = ENABLE;
    adc_descriptors[0].BTCTRL.bit.SRCINC = DISABLE;
    // Set beat size to a single byte
    adc_descriptors[0].BTCTRL.bit.BEATSIZE = DMAC_BTCTRL_BEATSIZE_BYTE_Val;
    // Instruct DMA to fire interrupt after every descriptor is complete
    adc_descriptors[0].BTCTRL.bit.BLOCKACT = DMAC_BTCTRL_BLOCKACT_INT_Val;
    // Flag descriptor as ready to go. Otherwise DMA will suspend
    adc_descriptors[0].BTCTRL.bit.VALID = ENABLE;


    // Prefill
    memcpy(&adc_descriptors[1], &adc_descriptors[0], sizeof(DmacDescriptor));
    // Setup next descriptor
    adc_descriptors[1].DESCADDR.bit.DESCADDR = (uint32_t) &adc_descriptors[2];
    // Store bytes to buffer
    adc_descriptors[1].DSTADDR.bit.DSTADDR = (uint32_t)buffers[1] + ADC_BUFFER_SIZE;


    // Prefill
    memcpy(&adc_descriptors[2], &adc_descriptors[0], sizeof(DmacDescriptor));
    // Setup next descriptor
    adc_descriptors[2].DESCADDR.bit.DESCADDR = (uint32_t) &adc_descriptors[0];
    // Store bytes to buffer
    adc_descriptors[2].DSTADDR.bit.DSTADDR = (uint32_t)buffers[2] + ADC_BUFFER_SIZE;
    

    // Provide first descriptor to DMA
    memcpy(&descriptor_section[DMA_CHANNEL],&adc_descriptors[0], sizeof(DmacDescriptor));
}

uint32_t getLastCompletedBufferId()
{
    const uint32_t reg = descriptor_writeback_section[DMA_CHANNEL].DSTADDR.reg - ADC_BUFFER_SIZE;
    return (reg - (uint32_t)buffers[0]) / ADC_BUFFER_SIZE;
}

void DMAC_Handler() {
    __disable_irq();

    // Get highest priority channel who created interrupts
    // DMA_CHANNEL == active_channel
    const uint8_t active_channel =  DMAC->INTPEND.bit.ID;

    // Activate channel (required for channel specific registers)
    DMAC->CHID.bit.ID = active_channel;
    
    // TODO: Check for error

    // Clear interrupt flags (write 1 to clear)
    DMAC->CHINTFLAG.bit.SUSP = 1;
    DMAC->CHINTFLAG.bit.TCMPL = 1;
    DMAC->CHINTFLAG.bit.TERR = 1;

    __enable_irq();
}

void mDMA::start()
{
    // Activate channel (required for channel specific registers)
    DMAC->CHID.bit.ID = DMA_CHANNEL;

    // Enable the DMA
    DMAC->CHCTRLA.bit.ENABLE = ENABLE;

    logInfo("Sampling stated");
}

void mDMA::resume()
{
    // Recreate descriptor chaining
    adc_descriptors[0].DESCADDR.bit.DESCADDR = (uint32_t)&adc_descriptors[1];
    adc_descriptors[1].DESCADDR.bit.DESCADDR = (uint32_t)&adc_descriptors[2];
    adc_descriptors[2].DESCADDR.bit.DESCADDR = (uint32_t)&adc_descriptors[0];

    start();
}

void mDMA::suspend()
{
    // Break descriptor chaining such that the DMA finishes after one more descriptor
    for(uint32_t i = 0;i<ADC_NUM_BUFFERS;i++)
    {
        adc_descriptors[i].DESCADDR.bit.DESCADDR = 0;
    }
    logInfo("Sampling stopping...");
}

uint32_t triggeredBuffer = 0;

void mDMA::triggerHit()
{
    logTrace("Trigger hit!");
    triggeredBuffer = getLastCompletedBufferId();
    dataWasSent = false;
    suspend();
}

void mDMA::init()
{
    logTrace("DMA: init");
    initDescriptors();
    initChannel();

    // enable DMA clocks
    PM->AHBMASK.bit.DMAC_ = ENABLE;
    PM->APBBMASK.bit.DMAC_ = ENABLE;

    // enable DMA interrupts
    NVIC_EnableIRQ( DMAC_IRQn ) ;

    // Connect DMA and allocated descriptors
    // These registers can only be written to when CTRL->DMAENABLE is DISABLED
    DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
    DMAC->WRBADDR.reg = (uint32_t)descriptor_writeback_section;

    // Enable DMA
    DMAC->CTRL.bit.DMAENABLE = ENABLE;

    // Set enable priority level 0
    DMAC->CTRL.bit.LVLEN0 = ENABLE;
}

void mDMA::handleEvents()
{
    // If last completed descriptor has not follower the sampling has finished
    if(dataWasSent == false && descriptor_writeback_section[DMA_CHANNEL].DESCADDR.bit.DESCADDR == 0)
    {
        // reorder buffers if neccessary
        if(triggeredBuffer == 1)
        {
            swap(buffers[0], buffers[1], ADC_BUFFER_SIZE);
            swap(buffers[1], buffers[2], ADC_BUFFER_SIZE);
        }
        else if(triggeredBuffer == 2)
        {
            swap(buffers[1], buffers[2], ADC_BUFFER_SIZE);
            swap(buffers[0], buffers[1], ADC_BUFFER_SIZE);
        }
        mNetwork::sendFragmentedSamples(buffers[0], ADC_NUM_BUFFERS * ADC_BUFFER_SIZE);
        dataWasSent = true;
        resume();
    }
}

uint8_t* mDMA::getLastCompletedBuffer()
{
    // If last completed descriptor has not follower the sampling has finished
    if(descriptor_writeback_section[DMA_CHANNEL].DESCADDR.bit.DESCADDR == 0)
    {
        return nullptr;
    }
    return buffers[getLastCompletedBufferId()];
}