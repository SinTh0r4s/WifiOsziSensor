#pragma once

#include "Arduino.h"


namespace mNetwork
{
    void handleEvents();
    void connectWifi();
    void init();
    void beginListen();
    void sendFragmentedSamples(const uint8_t* samples, uint32_t numSamples);
};