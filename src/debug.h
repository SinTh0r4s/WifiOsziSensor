#pragma once
#include "Arduino.h"

enum LogLevel : uint8_t
{
    NONE = 0,
    ERROR = 1,
    INFO = 2,
    DEBUG = 3,
    TRACE = 4
};

LogLevel _logLevel = LogLevel::NONE;
uint32_t _ledState = LOW;

inline void setLedHigh()
{
    digitalWrite(LED_BUILTIN, HIGH);
    _ledState = HIGH;
}

inline void setLedLow()
{
    digitalWrite(LED_BUILTIN, LOW);
    _ledState = LOW;
}

inline void toggleLed()
{
    if(_ledState)
        setLedLow();
    else
        setLedHigh();
}

inline void initLogging()
{
    pinMode(LED_BUILTIN, OUTPUT);
    setLedHigh();
    _logLevel = LogLevel::NONE;
#ifdef SERIAL_DEBUG
    Serial.begin(115200);
    // Halt execution until serial connection is established
    while(!Serial);
#endif
}

inline void setLogLevel(LogLevel logLevel)
{
    _logLevel = logLevel;
}

inline void _log(const char* msg)
{
#ifdef SERIAL_DEBUG
    Serial.print(msg);
#endif
}

inline void _logln(const char* msg)
{
#ifdef SERIAL_DEBUG
    Serial.println(msg);
#endif
}

inline void logError(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::ERROR)
    {
        _log("ERROR : ");
        _logln(msg);
    }
#endif
    while(true)
    {
        setLedLow();
        delay(1000);

        // morse s
        setLedHigh();
        delay(100);
        setLedLow();
        delay(100);
        setLedHigh();
        delay(100);
        setLedLow();
        delay(100);
        setLedHigh();
        delay(100);
        setLedLow();
        delay(300);

        //morse o
        setLedHigh();
        delay(300);
        setLedLow();
        delay(100);
        setLedHigh();
        delay(300);
        setLedLow();
        delay(100);
        setLedHigh();
        delay(300);
        setLedLow();
        delay(300);

        // morse s
        setLedHigh();
        delay(100);
        setLedLow();
        delay(100);
        setLedHigh();
        delay(100);
        setLedLow();
        delay(100);
        setLedHigh();
        delay(100);
    }
}

inline void logInfo(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::INFO)
    {
        _log("Info  : ");
        _logln(msg);
    }
#endif
}

inline void logDebug(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::DEBUG)
    {
        _log("DEBUG : ");
        _logln(msg);
    }
#endif
}

inline void logTrace(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::TRACE)
    {
        _log("TRACE : ");
        _logln(msg);
    }
#endif
}
