#pragma once

#include "Arduino.h"


typedef void (*setTriggerCallback) (uint8_t _channel, bool _active, uint32_t _mV);

void Network_registerSetTriggerCallback(setTriggerCallback _cb);
void Network_handleEvents();
void Network_connectWifi();
void Network_init();
void Network_beginListen();
void Network_sendSamples(uint8_t* samples, uint32_t numSamples);