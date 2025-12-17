#include "Log.h"

namespace iotsmartsys::core
{
    // default no-op logger to avoid null deref during static init
    namespace
    {
        class DefaultLogger : public ILogger
        {
        public:
            void logf(LogLevel level, const char *tag, const char *fmt, va_list args) override
            {
                // no-op to avoid using Serial before setup
                (void)level;
                (void)tag;
                (void)fmt;
                (void)args;
            }
        } defaultLogger;
    }

    ILogger *Log::_logger = nullptr;

    ILogger &Log::get()
    {
        return (_logger != nullptr) ? *_logger : static_cast<ILogger &>(defaultLogger);
    }

} // namespace iotsmartsys::core
