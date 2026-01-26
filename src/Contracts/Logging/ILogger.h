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

#ifndef IOTSMARTSYS_LOG_LEVEL
#define IOTSMARTSYS_LOG_LEVEL 2
#endif

#define IOTSMARTSYS_LOG_LEVEL_ERROR 0
#define IOTSMARTSYS_LOG_LEVEL_WARN 1
#define IOTSMARTSYS_LOG_LEVEL_INFO 2
#define IOTSMARTSYS_LOG_LEVEL_DEBUG 3
#define IOTSMARTSYS_LOG_LEVEL_TRACE 4

#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_TRACE
#define IOT_LOG_TRACE(logger, tag, fmt, ...) (logger).trace(tag, fmt, ##__VA_ARGS__)
#else
#define IOT_LOG_TRACE(logger, tag, fmt, ...) do { (void)(logger); } while (0)
#endif

#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_DEBUG
#define IOT_LOG_DEBUG(logger, tag, fmt, ...) (logger).debug(tag, fmt, ##__VA_ARGS__)
#else
#define IOT_LOG_DEBUG(logger, tag, fmt, ...) do { (void)(logger); } while (0)
#endif

#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_INFO
#define IOT_LOG_INFO(logger, tag, fmt, ...) (logger).info(tag, fmt, ##__VA_ARGS__)
#else
#define IOT_LOG_INFO(logger, tag, fmt, ...) do { (void)(logger); } while (0)
#endif

#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_WARN
#define IOT_LOG_WARN(logger, tag, fmt, ...) (logger).warn(tag, fmt, ##__VA_ARGS__)
#else
#define IOT_LOG_WARN(logger, tag, fmt, ...) do { (void)(logger); } while (0)
#endif

#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_ERROR
#define IOT_LOG_ERROR(logger, tag, fmt, ...) (logger).error(tag, fmt, ##__VA_ARGS__)
#else
#define IOT_LOG_ERROR(logger, tag, fmt, ...) do { (void)(logger); } while (0)
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
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_TRACE
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Trace, tag, fmt, args);
            va_end(args);
#endif
        }

        void trace(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_TRACE
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Trace, "", fmt, args);
            va_end(args);
#endif
        }

        // Helpers comuns
        void error(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_ERROR
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Error, tag, fmt, args);
            va_end(args);
#endif
        }
        void error(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_ERROR
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Error, "", fmt, args);
            va_end(args);
#endif
        }

        void warn(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_WARN
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Warn, tag, fmt, args);
            va_end(args);
#endif
        }

        void warn(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_WARN
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Warn, "", fmt, args);
            va_end(args);
#endif
        }

        void info(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_INFO
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Info, tag, fmt, args);
            va_end(args);
#endif
        }

        void info(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_INFO
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Info, "", fmt, args);
            va_end(args);
#endif
        }

        void debug(const char *tag, const char *fmt, ...) IOT_PRINTF_FMT(3, 4)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_DEBUG
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Debug, tag, fmt, args);
            va_end(args);
#endif
        }

        void debug(const char *fmt, ...) IOT_PRINTF_FMT(2, 3)
        {
#if IOTSMARTSYS_LOG_LEVEL >= IOTSMARTSYS_LOG_LEVEL_DEBUG
            va_list args;
            va_start(args, fmt);
            logf(LogLevel::Debug, "", fmt, args);
            va_end(args);
#endif
        }
    };

} // namespace iotsmartsys::core
