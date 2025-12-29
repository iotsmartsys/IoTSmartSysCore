#pragma once
#include <cstdarg>
#include <cstdint>

namespace iotsmartsys::core
{

    enum class LogLevel : std::uint8_t
    {
        Error = 0,
        Warn = 1,
        Info = 2,
        Debug = 3,
        Trace = 4
    };

    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        // Log "printf-like" sem alocar memória
        virtual void logf(LogLevel level, const char *tag, const char *fmt, va_list args) = 0;

        // Conveniência: variádico
        void log(LogLevel level, const char *tag, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            logf(level, tag, fmt, args);
            va_end(args);
        }

        void trace(const char *tag, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Trace, tag, fmt, args);
            va_end(args);
        }

        void trace(const char *fmt, ...)
        {
            trace("", fmt);
        }

        // Helpers comuns
        void error(const char *tag, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Error, tag, fmt, args);
            va_end(args);
        }
        void error(const char *fmt, ...)
        {
            error("", fmt);
        }

        void warn(const char *tag, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Warn, tag, fmt, args);
            va_end(args);
        }

        void warn(const char *fmt, ...)
        {
            warn("", fmt);
        }

        void info(const char *tag, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Info, tag, fmt, args);
            va_end(args);
        }

        void info(const char *fmt, ...)
        {
            info("", fmt);
        }

        void debug(const char *tag, const char *fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Debug, tag, fmt, args);
            va_end(args);
        }

        void debug(const char *fmt, ...)
        {
            debug("", fmt);
        }
    };

} // namespace iotsmartsys::core