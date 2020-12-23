// libs
#include "Arduino.h"

// local includes
#include "debug.h"
#include "constants.h"
#include "network.h"
#include "adc.h"


void setTrigger(uint8_t _channel, bool _active, uint32_t _mV)
{
    if(_channel > BOARD_CHANNELS)
        return;
    logTrace("Set trigger");
    Adc_setTrigger(_channel, _active, _mV);
}

void sendAdcBuffer(const uint8_t* buffer, uint32_t numSamples)
{
    Network_sendSamples(buffer, numSamples);
}

void setup()
{
    startInit();
    setLogLevel(LogLevel::TRACE);

    Adc_init();
    Adc_registerSendBufferCallback(sendAdcBuffer);
    
    Network_init();
    Network_connectWifi();
    Network_registerSetTriggerCallback(setTrigger);
    Network_beginListen();

    endInit();
}

void loop()
{
    Network_handleEvents();
    Adc_handleEvents();
}