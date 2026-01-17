#pragma once

#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Settings/Settings.h"
#include "Contracts/Settings/SettingsManager.h"
#include "Core/Providers/ServiceManager.h"

namespace iotsmartsys::app
{
    class ConnectivityBootstrap
    {
    public:
        enum class BootPath : uint8_t
        {
            Wifi = 0,
            Provisioning = 1
        };

        ConnectivityBootstrap(core::ILogger &logger,
                              core::ServiceManager &serviceManager,
                              core::settings::SettingsManager &settingsManager,
                              core::WiFiManager &wifi);

        BootPath run(core::settings::Settings &outSettings);

    private:
        void logSettingsSummary(const core::settings::Settings &settings) const;

        core::ILogger &logger_;
        core::ServiceManager &serviceManager_;
        core::settings::SettingsManager &settingsManager_;
        core::WiFiManager &wifi_;
    };
} // namespace iotsmartsys::app
