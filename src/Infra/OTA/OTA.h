#ifndef OTA_DISABLED
#pragma once

#include "Contracts/Logging/ILogger.h"
#include "Contracts/Providers/IDeviceIdentityProvider.h"

using namespace iotsmartsys::core;
namespace iotsmartsys::ota
{
    class OTA
    {
    public:
        OTA(ILogger &logger, IDeviceIdentityProvider &deviceIdentityProvider);

        void setup();

        void handle();
        bool isInitialized() const { return _initialized; }

    private:
        ILogger &_logger;

        core::IDeviceIdentityProvider &_deviceIdentityProvider;
        bool _initialized = false;
    };
}
#endif