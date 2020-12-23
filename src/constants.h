#pragma once

#include "Arduino.h"
#include <WiFi101.h>


// Board settings
static const char* BOARD_DESCRIPTION = "Arduino MKR1000";
static const char* ADC_DESCRIPTION = "integrated";

// General settings
static const uint32_t BOARD_LISTENING_PORT = 7567;
static const uint32_t UI_LISTENING_PORT = 7567;
static const uint32_t BEACON_SEPARATION_TIME = 1000;    // in msec


// Random constants
static const uint32_t ETHERNET_MAX_BUFFER_SIZE = 1518;
static const uint16_t MAGIC_ID = 0x7567;
static const IPAddress BROADCAST_IP = {255, 255, 255, 255};