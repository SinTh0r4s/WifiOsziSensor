#pragma once

#include "Arduino.h"

struct __attribute__((__packed__)) Beacon
{
    uint16_t magicNumber;   // 0x7567 to distinguish from random UDP traffic that might hit that port
    uint8_t resolution;     // how many bits per sample
    uint8_t channels;       // simple counter
    char model[30];
    char adc[30];
    uint32_t frequency;     // in Hz
    uint32_t numSamples;    // simple counter
    float sampleTime;       // in sec
    uint32_t v_ref;         // in mV
    uint32_t port;          // UDP Port of board
};
