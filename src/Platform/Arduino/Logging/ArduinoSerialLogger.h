#pragma once
#include <Arduino.h>
#include <cstdarg>

#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::platform::arduino
{

    class ArduinoSerialLogger final : public iotsmartsys::core::ILogger
    {
    public:
        explicit ArduinoSerialLogger(Print &out, iotsmartsys::core::LogLevel minLevel = iotsmartsys::core::LogLevel::Info)
            : _out(out), _minLevel(minLevel) {}

        void setMinLevel(iotsmartsys::core::LogLevel lvl) { _minLevel = lvl; }

        void logf(iotsmartsys::core::LogLevel level, const char *tag, const char *fmt, va_list args) override;

    private:
        Print &_out;
        iotsmartsys::core::LogLevel _minLevel;
    };

} // namespace iotsmartsys::platform::arduino