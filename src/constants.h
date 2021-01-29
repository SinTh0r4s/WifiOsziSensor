#pragma once

#include "Arduino.h"
#ifdef ESP32
    #include <WiFi.h>
#elif MKR1000
    #include <WiFi101.h>
#endif

#ifdef MKR1000

// Board settings
static const char* BOARD_DESCRIPTION = "Arduino MKR1000";
static const char* BOARD_ADC_DESCRIPTION = "integrated";
static const uint8_t BOARD_CHANNELS = 1;
static const uint32_t DMA_CHANNEL = 0;
static const uint32_t ADC_BUFFER_SIZE = 7000;
static const uint32_t ADC_NUM_BUFFERS = 3;
static const uint8_t BOARD_RESOLUTION = 8;
static const uint32_t BOARD_V_REF = 1650;
static const uint32_t ADC_FREQUENCY = 100000;
static const uint32_t BOARD_SPI_CS = 15;

#endif
#ifdef ESP32

// Board settings
static const char* BOARD_DESCRIPTION = "ESP32 DevkitC";
static const char* BOARD_ADC_DESCRIPTION = "MAX11131";
static const uint8_t BOARD_CHANNELS = 1;
static const uint32_t ADC_BUFFER_SIZE = 10000;
static const uint32_t ADC_NUM_BUFFERS = 1;
static const uint8_t BOARD_RESOLUTION = 12;
static const uint32_t BOARD_V_REF = 3300;
static const uint32_t ADC_FREQUENCY = 580000;
static const uint32_t BOARD_SPI_CS = 5;

#endif


// General settings
static const uint32_t BOARD_LISTENING_PORT = 7567;
static const uint32_t BEACON_LISTENING_PORT = 7567;
static const uint32_t BEACON_SEPARATION_TIME = 1000;    // in msec
static const uint32_t BYTES_PER_PACKET = 1024;


// Random constants
static const uint32_t ETHERNET_MAX_BUFFER_SIZE = 1518;
static const uint16_t MAGIC_ID = 0x7567;
static const IPAddress BROADCAST_IP = {255, 255, 255, 255};
static const uint32_t BYTES_PER_SAMPLE = (BOARD_RESOLUTION + 7) / 8;
static const uint32_t SAMPLES_PER_PACKET = BYTES_PER_PACKET / BYTES_PER_SAMPLE;


#if BYTES_PER_SAMPLE * SAMPLES_PER_PACKET != BYTES_PER_PACKET
    #error Packet size must match sample size and resolution without remainder
#endif