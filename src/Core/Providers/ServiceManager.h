#pragma once

#include "Contracts/Logging/Log.h"
#include "Contracts/Firmware/IFirmwareRuntimeInfo.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Settings/SettingsManager.h"

namespace iotsmartsys::core
{
    class ServiceManager
    {
    public:
        static ServiceManager &init();
        static ServiceManager &instance();

        ILogger &logger();
        ITimeProvider &timeProvider();
        settings::SettingsManager &settingsManager();
        settings::ISettingsGate &settingsGate();
        settings::IReadOnlySettingsProvider &settingsProvider();
        core::WiFiManager &wifiManager();
        firmware::IFirmwareRuntimeInfo *firmwareRuntimeInfo();

        void setLogLevel(LogLevel level);

    private:
        ServiceManager();
        void registerServices();

        ServiceProvider &serviceProvider_;
    };
} // namespace iotsmartsys::core
