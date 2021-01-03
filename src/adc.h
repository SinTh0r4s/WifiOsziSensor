#pragma once

#include "Arduino.h"

namespace mADC
{
    void init();
    void setTrigger(uint8_t _channel, bool _active, uint32_t _mV);
    void handleEvents();
};