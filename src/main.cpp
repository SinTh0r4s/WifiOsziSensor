#define SERIAL_DEBUG 1

// libs
#include "Arduino.h"
#include <WiFi101.h>
#include <WiFiUdp.h>

// local includes
#include "secrets.h"


inline void LOG(const char* s)
{
#ifdef SERIAL_DEBUG
    Serial.println(s);
#endif
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
#ifdef SERIAL_DEBUG
    Serial.begin(115200);
    while(!Serial);
#endif
}

void loop()
{
    
}