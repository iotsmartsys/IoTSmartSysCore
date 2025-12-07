#include "Logger.h"

#ifndef LOG_DISABLED

#include <Arduino.h>

Print *Logger::_sink = nullptr;
bool Logger::enabled = true;
bool Logger::defaultInitDone = false;
LogLevel Logger::minLevel = static_cast<LogLevel>(LOG_DEFAULT_LEVEL);

// Optional hook for custom sink. Define LOG_OUTPUT_CUSTOM and provide this symbol in your firmware.
extern "C" Print *Logger_getCustomSink() __attribute__((weak));

void Logger::initDefaultIfNeeded()
{
    if (defaultInitDone) return;
    defaultInitDone = true;

#if defined(LOG_OUTPUT_SERIAL)
    _sink = &Serial;
#elif defined(LOG_OUTPUT_CUSTOM)
    if (Logger_getCustomSink) {
        _sink = Logger_getCustomSink();
    }
#else
    // Fallback: Serial
    _sink = &Serial;
#endif
}

void Logger::begin(unsigned long baud)
{
#if defined(LOG_OUTPUT_SERIAL) || !defined(LOG_OUTPUT_CUSTOM)
    Serial.begin(baud);
#else
    (void)baud; // unused for custom sinks
#endif
}

void Logger::setEnabled(bool en)
{
    enabled = en;
}

bool Logger::isEnabled()
{
    return enabled;
}

void Logger::setMinLevel(LogLevel level)
{
    minLevel = level;
}

void Logger::setSink(Print *sink)
{
    _sink = sink;
    defaultInitDone = true;
}

void Logger::printf(const char *fmt, ...)
{
    if (!enabled) return; initDefaultIfNeeded(); if (!_sink) return;

    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    _sink->print(buffer);
}

#endif // LOG_DISABLED
