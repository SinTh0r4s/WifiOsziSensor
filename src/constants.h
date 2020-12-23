#pragma once

#include "Arduino.h"
#include <WiFi101.h>


// Board settings
static const char* BOARD_DESCRIPTION = "Arduino MKR1000";
static const char* BOARD_ADC_DESCRIPTION = "integrated";
static const uint8_t BOARD_CHANNELS = 1;
static const uint32_t DMA_CHANNEL = 0;
static const uint32_t ADC_BUFFER_SIZE = 10240;
static const uint8_t BOARD_RESOLUTION = 8;
static const uint32_t BOARD_V_REF = 1650;

// General settings
static const uint32_t BOARD_LISTENING_PORT = 7567;
static const uint32_t BEACON_LISTENING_PORT = 7567;
static const uint32_t BEACON_SEPARATION_TIME = 1000;    // in msec
static const uint32_t SAMPLES_PER_PACKET = 1024;


// Random constants
static const uint32_t ETHERNET_MAX_BUFFER_SIZE = 1518;
static const uint16_t MAGIC_ID = 0x7567;
static const IPAddress BROADCAST_IP = {255, 255, 255, 255};