#pragma once
#include <cstdarg>
#include <cstdint>

#if defined(__GNUC__)
    #define IOT_PRINTF_FMT(fmtIndex, firstArg) __attribute__((format(printf, fmtIndex, firstArg)))
    // Para funções estilo vprintf (va_list)
    #define IOT_VPRINTF_FMT(fmtIndex) __attribute__((format(printf, fmtIndex, 0)))
#else
    #define IOT_PRINTF_FMT(fmtIndex, firstArg)
    #define IOT_VPRINTF_FMT(fmtIndex)
#endif

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
        virtual void logf(LogLevel level, const char *tag, const char *fmt, va_list args) IOT_VPRINTF_FMT(4) = 0;
        virtual void setMinLevel(LogLevel) {}

        // Conveniência: variádico
        void log(LogLevel level, const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(4, 5)
        {
            va_list args;
            va_start(args, fmt);
            logf(level, tag, fmt, args);
            va_end(args);
        }

        void trace(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Trace, tag, fmt, args);
            va_end(args);
        }

        void trace(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Trace, "", fmt, args);
            va_end(args);
        }

        // Helpers comuns
        void error(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Error, tag, fmt, args);
            va_end(args);
        }
        void error(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Error, "", fmt, args);
            va_end(args);
        }

        void warn(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Warn, tag, fmt, args);
            va_end(args);
        }

        void warn(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Warn, "", fmt, args);
            va_end(args);
        }

        void info(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Info, tag, fmt, args);
            va_end(args);
        }

        void info(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Info, "", fmt, args);
            va_end(args);
        }

        void debug(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Debug, tag, fmt, args);
            va_end(args);
        }

        void debug(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Debug, "", fmt, args);
            va_end(args);
        }
    };

} // namespace iotsmartsys::core
