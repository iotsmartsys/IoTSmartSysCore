#pragma once

#include "Contracts/Display/ScreenColor.h"

#include <cstdarg>

namespace iotsmartsys::core
{
    class IScreenConsole
    {
    public:
        virtual ~IScreenConsole() = default;

        virtual void begin() = 0;
        virtual void writef(ScreenColor color, const char *fmt, va_list args) = 0;
        virtual void clear() = 0;
        virtual bool isReady() const = 0;

        void write(ScreenColor color, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            writef(color, fmt, args);
            va_end(args);
        }

        void info(const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            writef(ScreenColor::White, fmt, args);
            va_end(args);
        }

        void warn(const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            writef(ScreenColor::Yellow, fmt, args);
            va_end(args);
        }

        void error(const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            writef(ScreenColor::Red, fmt, args);
            va_end(args);
        }
    };
} // namespace iotsmartsys::core
