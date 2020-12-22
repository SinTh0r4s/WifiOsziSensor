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

inline void init_logging()
{
    _logLevel = LogLevel::NONE;
#ifdef SERIAL_DEBUG
    Serial.begin(115200);
#endif
}

inline void setLogLevel(LogLevel logLevel)
{
    _logLevel = logLevel;
}

inline void _log(const char* msg)
{
#ifdef SERIAL_DEBUG
    Serial.println(msg);
#endif
}

inline void logError(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::ERROR)
        _log(strcat("ERROR : ", msg));
#endif
}

inline void logInfo(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::INFO)
        _log(strcat("Info  : ", msg));
#endif
}

inline void logDebug(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::DEBUG)
        _log(strcat("DEBUG : ", msg));
#endif
}

inline void logTrace(const char* msg)
{
#ifdef SERIAL_DEBUG
    if(_logLevel >= LogLevel::TRACE)
        _log(strcat("TRACE : ", msg));
#endif
}
