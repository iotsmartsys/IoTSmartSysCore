#pragma once
#include "Contracts/Logging/ILogger.h"

using namespace iotsmartsys::core;
namespace iotsmartsys::ota
{
    class OTA
    {
    public:
        OTA(ILogger &logger);

        void setup();

        void handle();
        bool isInitialized() const { return _initialized; }

    private:
        ILogger &_logger;
        bool _initialized = false;
    };
}