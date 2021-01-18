#ifdef ESP32


#include "adc.h"
#include "debug.h"
#include "constants.h"
#include "network.h"
#include <SPI.h>


SPIClass vspi(VSPI);

uint16_t spi_master_rx_buf[ADC_BUFFER_SIZE];
int32_t idx = 0;
int32_t endIdx = 0;
volatile uint16_t triggerValue = 0;
volatile bool triggerActive = false;
bool triggerHit = false;
const uint32_t MILLION = 1000 * 1000;


inline uint16_t transfer16(uint16_t value)
{
    digitalWrite(BOARD_SPI_CS, LOW);
    const uint16_t retVal = vspi.transfer16(value);
    digitalWrite(BOARD_SPI_CS, HIGH);
    return retVal;
}

void mADC::init()
{
    // HSPI = CS: 15, CLK: 14, MOSI: 13, MISO: 12
    // VSPI = CS: 5, CLK: 18, MOSI: 23, MISO: 19
    vspi.begin();
    vspi.beginTransaction(SPISettings(8 * MILLION, MSBFIRST, SPI_MODE3));  // SPI_MASTER_FREQ_40M
    pinMode(BOARD_SPI_CS, OUTPUT); // Overwrite hardware CS handling

    // Configure ADC
    {
        // ADC MODE CONTROL: reset
        transfer16(0x0040);
        // ADC Configuration register: set reference (single-end)
        transfer16(0x8004);
        // UNIPOLAR register: single-end
        transfer16(0x8800);
        // BIPOLAR register: single-end
        transfer16(0x9000);
        // RANGE register: single-end
        transfer16(0x9800);
        // SCAN0 register: single-end
        transfer16(0xA000);
        // SCAN1 register: single-end
        // 1010 1000 0100 0000 = 0xA840 Scan CH3
        transfer16(0xA840);
        // CUSTOM_EXT CHAN_ID
        transfer16(0x4304);
    }
}

void mADC::setTrigger(uint8_t _channel, bool _active, uint32_t _mV)
{
    triggerValue = _mV;
    triggerActive = _active;
}

void mADC::handleEvents()
{
    if(triggerHit)
    {
        if(idx == endIdx)
        {
            mNetwork::sendFragmentedSamples((uint8_t*)buffer, ADC_BUFFER_SIZE * sizeof(uint16_t));
            triggerHit = false;
        }
    }

    static uint16_t lastValue = 0;
    const uint16_t value = transfer16(0x0000);
    spi_master_rx_buf[idx] = value;

    if(triggerActive)
    {
        if(lastValue < triggerValue && value >= triggerValue || lastValue > triggerValue && value <= triggerValue)
        {
            endIdx = (idx - 1) % ADC_BUFFER_SIZE;
            triggerHit = true;
            triggerActive = false;
        }
    }

    lastValue = value;
    idx++;
    idx %= ADC_BUFFER_SIZE;
}


#endif  // ESP32