#define SERIAL_DEBUG 1


#include "Arduino.h"
#include <WiFi101.h>


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