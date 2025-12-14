#pragma once

#include "ILogger.h"

namespace iotsmartsys::core
{

    class Log
    {
    public:
        static void setLogger(ILogger *logger) { _logger = logger; }
        static ILogger &get() { return *_logger; }

    private:
        inline static ILogger *_logger = nullptr;
    };

}