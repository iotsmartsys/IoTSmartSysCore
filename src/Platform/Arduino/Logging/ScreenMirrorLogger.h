#pragma once

#include "Contracts/Display/IScreenConsole.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::platform::arduino
{
    class ScreenMirrorLogger final : public iotsmartsys::core::ILogger
    {
    public:
        ScreenMirrorLogger(
            iotsmartsys::core::ILogger &decorated,
            iotsmartsys::core::IScreenConsole &console,
            iotsmartsys::core::LogLevel minLevel = iotsmartsys::core::LogLevel::Info);

        void logf(
            iotsmartsys::core::LogLevel level,
            const char *tag,
            const char *fmt,
            va_list args) override;
        void setMinLevel(iotsmartsys::core::LogLevel level) override;

    private:
        static iotsmartsys::core::ScreenColor colorFor(iotsmartsys::core::LogLevel level);
        static const char *levelName(iotsmartsys::core::LogLevel level);

        iotsmartsys::core::ILogger &decorated_;
        iotsmartsys::core::IScreenConsole &console_;
        iotsmartsys::core::LogLevel minLevel_;
    };
} // namespace iotsmartsys::platform::arduino
