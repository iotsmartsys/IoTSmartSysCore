#pragma once

#include "Contracts/Display/IScreenConsole.h"

namespace iotsmartsys::core
{
    class NoOpScreenConsole final : public IScreenConsole
    {
    public:
        void begin() override {}
        void writef(ScreenColor color, const char *fmt, va_list args) override
        {
            (void)color;
            (void)fmt;
            (void)args;
        }
        void clear() override {}
        bool isReady() const override { return false; }
    };

    class Screen
    {
    public:
        static void setConsole(IScreenConsole *console) { _console = console; }
        static IScreenConsole &get();

    private:
        static IScreenConsole *_console;
    };
} // namespace iotsmartsys::core
