#pragma once

#include "Contracts/Display/Screen.h"
#include "Contracts/Logging/Log.h"
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
        IScreenConsole &screenConsole();
        ITimeProvider &timeProvider();
        settings::SettingsManager &settingsManager();
        settings::ISettingsGate &settingsGate();
        settings::IReadOnlySettingsProvider &settingsProvider();
        core::WiFiManager &wifiManager();
        providers::IBinaryCapabilityStateProvider *binaryCapabilityStateProvider();

        // BCS-024/BCS-029: activates the single asynchronous binary-state
        // writer. Only valid once the graph is fully built and the boot snapshot
        // has been read, and before any capability can request persistence.
        common::StateResult activateBinaryStateWriter();

        void setLogLevel(LogLevel level);

    private:
        ServiceManager();
        void registerServices();

        ServiceProvider &serviceProvider_;
    };
} // namespace iotsmartsys::core
