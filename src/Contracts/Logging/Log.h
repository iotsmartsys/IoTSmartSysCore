#pragma once

#include "ILogger.h"

namespace iotsmartsys::core
{

    class Log
    {
    public:
        static void setLogger(ILogger *logger) { _logger = logger; }
        static ILogger &get();

    private:
        static ILogger *_logger;
    };

}