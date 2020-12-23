#pragma once

#include "Arduino.h"

typedef void (*sendBufferCallback) (const uint8_t* _buffer, uint32_t _numBuffers);

void Adc_registerSendBufferCallback(sendBufferCallback _cb);
void Adc_init();
void Adc_setTrigger(uint8_t _channel, bool _active, uint32_t _mV);
void Adc_handleEvents();