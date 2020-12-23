// libs
#include "Arduino.h"

// local includes
#include "debug.h"
#include "constants.h"
#include "network.h"


void setTrigger(uint8_t _channel, bool _active, uint32_t _mV)
{
    if(_channel != 1)
        return;
    logWarning("setTrigger() not implemented yet!");
}

void setup()
{
    startInit();
    setLogLevel(LogLevel::TRACE);

    Network_connectWifi();
    Network_registerSetTriggerCallback(setTrigger);
    Network_beginListen();

    endInit();
}

void loop()
{
    Network_handleEvents();
}