#include "Platform/Arduino/Logging/ScreenMirrorLogger.h"

#include <cstdio>

namespace iotsmartsys::platform::arduino
{
    ScreenMirrorLogger::ScreenMirrorLogger(
        iotsmartsys::core::ILogger &decorated,
        iotsmartsys::core::IScreenConsole &console,
        iotsmartsys::core::LogLevel minLevel)
        : decorated_(decorated),
          console_(console),
          minLevel_(minLevel)
    {
        decorated_.setMinLevel(minLevel_);
    }

    void ScreenMirrorLogger::logf(
        iotsmartsys::core::LogLevel level,
        const char *tag,
        const char *fmt,
        va_list args)
    {
        va_list decoratedArgs;
        va_copy(decoratedArgs, args);
        decorated_.logf(level, tag, fmt, decoratedArgs);
        va_end(decoratedArgs);

        if (static_cast<std::uint8_t>(level) > static_cast<std::uint8_t>(minLevel_) ||
            fmt == nullptr)
        {
            return;
        }

        char message[160];
        va_list screenArgs;
        va_copy(screenArgs, args);
        const int result = vsnprintf(message, sizeof(message), fmt, screenArgs);
        va_end(screenArgs);
        if (result < 0)
        {
            return;
        }

        if (tag != nullptr && tag[0] != '\0')
        {
            console_.write(colorFor(level), "[%s] %s: %s", levelName(level), tag, message);
        }
        else
        {
            console_.write(colorFor(level), "[%s] %s", levelName(level), message);
        }
    }

    void ScreenMirrorLogger::setMinLevel(iotsmartsys::core::LogLevel level)
    {
        minLevel_ = level;
        decorated_.setMinLevel(level);
    }

    iotsmartsys::core::ScreenColor ScreenMirrorLogger::colorFor(iotsmartsys::core::LogLevel level)
    {
        using iotsmartsys::core::LogLevel;
        using iotsmartsys::core::ScreenColor;

        switch (level)
        {
        case LogLevel::Error:
            return ScreenColor::Red;
        case LogLevel::Warn:
            return ScreenColor::Yellow;
        case LogLevel::Info:
            return ScreenColor::White;
        case LogLevel::Debug:
        case LogLevel::Trace:
            return ScreenColor::Cyan;
        }
        return ScreenColor::Default;
    }

    const char *ScreenMirrorLogger::levelName(iotsmartsys::core::LogLevel level)
    {
        using iotsmartsys::core::LogLevel;
        switch (level)
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
        }
        return "UNK";
    }
} // namespace iotsmartsys::platform::arduino
