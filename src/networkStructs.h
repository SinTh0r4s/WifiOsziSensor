#pragma once

#include "Arduino.h"

struct __attribute__((__packed__)) BeaconHeader
{
    uint16_t magicNumber;       // 0x7567 to distinguish from random UDP traffic that might hit that port
    uint8_t resolution;         // how many bits per sample
    uint8_t channels;           // simple counter
    uint8_t beaconId;           // incrementing id for identification
    char model[30];             // string description with 30 characters max
    char adc[30];               // string description with 30 characters max
    uint32_t frequency;         // in Hz
    uint32_t numSamples;        // simple counter
    float sampleTime;           // in sec
    uint32_t v_ref;             // in mV
    uint32_t port;              // UDP Port of board
    uint16_t uid;               // Unique ID to distinguish identical boards. Last two bytes from MAC address
};

struct __attribute__((__packed__)) CommandHeader
{
    uint16_t magicNumber;       // 0x7567 to distinguish from random UDP traffic that might hit that port
    uint32_t port;              // which port to reply to
    uint8_t numSettings;        // how many settings are supplied
};

struct __attribute__((__packed__)) TriggerSettingHeader
{
    uint8_t channel;            // which channel these settings are valid
    uint32_t triggerVoltage;    // in mV
    uint8_t active;             // boolean, 0 = off, 1 = on
};

struct __attribute__((__packed__)) SampleTransmissionHeader
{
    uint16_t magicNumber;       // 0x7567 to distinguish from random UDP traffic that might hit that port
    uint8_t frameId;            // which frame is this
    uint8_t numFrames;          // count of frames per sample transmission
    uint8_t transmissionGroupId;// incrementing id
    uint8_t resolution;         // how many bits per sample
    uint8_t channels;           // which channels are transmitted: bitfield 1...8
    uint32_t frequency;         // in Hz
    uint32_t v_ref;             // in mV
    uint32_t numSamples;        // simple counter
};
