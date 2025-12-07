#pragma once

#include <Arduino.h>
#include <stdarg.h>

// Build-time selection examples (add to platformio.ini -> build_flags):
//  -D LOG_DISABLED            -> remove logs at compile-time
//  -D LOG_OUTPUT_SERIAL       -> usa Serial como destino padrão
//  -D LOG_OUTPUT_CUSTOM       -> você fornece um Print* em tempo de execução (Logger::setSink)
//  -D LOG_DEFAULT_LEVEL=2     -> 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL 0
#endif

enum class LogLevel : uint8_t { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3, NONE = 255 };

#ifdef LOG_DISABLED

class Logger {
public:
    static inline void begin(unsigned long) {}
    static inline void setEnabled(bool) {}
    static inline bool isEnabled() { return false; }
    static inline void setMinLevel(LogLevel) {}
    static inline void setSink(Print *) {}
    template<typename T> static inline void print(const T &) {}
    template<typename T> static inline void println(const T &) {}
    static inline void printf(const char *, ...) {}
    template<typename T> static inline void debug(const T &) {}
    template<typename T> static inline void info(const T &) {}
    template<typename T> static inline void warn(const T &) {}
    template<typename T> static inline void error(const T &) {}
};

#else

class Logger {
public:
    static void begin(unsigned long baud = 115200);
    static void setEnabled(bool en);
    static bool isEnabled();
    static void setMinLevel(LogLevel level);
    static void setSink(Print *sink);

    template<typename T>
    static inline void print(const T &val) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->print(val);
    }

    // Overloads for numeric + precision/base like Arduino Serial
    static inline void print(double val, int digits) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->print(val, digits);
    }
    static inline void println(double val, int digits) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->println(val, digits);
    }
    static inline void print(float val, int digits) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->print(val, digits);
    }
    static inline void println(float val, int digits) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->println(val, digits);
    }
    static inline void print(unsigned long val, int base) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->print(val, base);
    }
    static inline void println(unsigned long val, int base) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->println(val, base);
    }

    template<typename T>
    static inline void println(const T &val) {
        if (!enabled) return; initDefaultIfNeeded(); if (_sink) _sink->println(val);
    }

    static void printf(const char *fmt, ...);

    template<typename T>
    static inline void debug(const T &msg) { logWithLevel(LogLevel::DEBUG, msg); }
    template<typename T>
    static inline void info(const T &msg)  { logWithLevel(LogLevel::INFO,  msg); }
    template<typename T>
    static inline void warn(const T &msg)  { logWithLevel(LogLevel::WARN,  msg); }
    template<typename T>
    static inline void error(const T &msg) { logWithLevel(LogLevel::ERROR, msg); }

private:
    static void initDefaultIfNeeded();
    template<typename T>
    static inline void logWithLevel(LogLevel level, const T &msg) {
        if (!enabled) return; initDefaultIfNeeded();
        if ((uint8_t)level < (uint8_t)minLevel) return;
        if (!_sink) return;
        switch (level) {
            case LogLevel::DEBUG: _sink->print("[DEBUG] "); break;
            case LogLevel::INFO:  _sink->print("[INFO]  "); break;
            case LogLevel::WARN:  _sink->print("[WARN]  "); break;
            case LogLevel::ERROR: _sink->print("[ERROR] "); break;
            default: break;
        }
        _sink->println(msg);
    }

    static Print *_sink;
    static bool enabled;
    static bool defaultInitDone;
    static LogLevel minLevel;
};

#endif // LOG_DISABLED

// Convenience wrappers (opcionais) — mapeiam para a classe Logger
#define LOG_BEGIN(baud)      do { Logger::begin(baud); } while(0)
#define LOG_PRINT(x)         do { Logger::print(x); } while(0)
#define LOG_PRINTLN(x)       do { Logger::println(x); } while(0)
#define LOG_PRINTF(...)      do { Logger::printf(__VA_ARGS__); } while(0)
#define LOG_DEBUG(x)         do { Logger::debug(x); } while(0)
#define LOG_INFO(x)          do { Logger::info(x); } while(0)
#define LOG_WARN(x)          do { Logger::warn(x); } while(0)
#define LOG_ERROR(x)         do { Logger::error(x); } while(0)
