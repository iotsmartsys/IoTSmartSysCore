#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include <stdio.h>

namespace iotsmartsys::platform::arduino
{

    static const char *levelToStr(iotsmartsys::core::LogLevel lvl)
    {
        using iotsmartsys::core::LogLevel;
        switch (lvl)
        {
        case LogLevel::Error:
            return "ERR";
        case LogLevel::Warn:
            return "WRN";
        case LogLevel::Info:
            return "INF";
        case LogLevel::Debug:
            return "DBG";
        case LogLevel::Trace:
            return "TRC";
        default:
            return "UNK";
        }
    }

    void ArduinoSerialLogger::logf(iotsmartsys::core::LogLevel level, const char *tag, const char *fmt, va_list args)
    {
        if ((std::uint8_t)level > (std::uint8_t)_minLevel)
            return;

        char buf[160]; // ajuste se quiser (tradeoff RAM x truncamento)
        int n = vsnprintf(buf, sizeof(buf), fmt, args);
        if (n < 0)
            return;

        _out.print('[');
        _out.print(levelToStr(level));
        _out.print(']');
        if (tag && tag[0])
        {
            _out.print(' ');
            _out.print(tag);
            _out.print(':');
        }
        _out.print(' ');
        _out.println(buf);
    }

} // namespace iotsmartsys::platform::arduino