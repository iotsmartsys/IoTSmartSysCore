#pragma once

#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Settings/SettingsManager.h"
#include "Core/Settings/SettingsGateImpl.h"

#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Platform/Espressif/Settings/EspIdfSettingsFetcher.h"
#include "Platform/Espressif/Settings/EspIdfSettingsParser.h"
#include "Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.h"

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

        void setLogLevel(LogLevel level);

    private:
        ServiceManager();
        void registerServices();

        platform::arduino::ArduinoSerialLogger logger_;
        platform::arduino::ArduinoTimeProvider timeProvider_;
        ServiceProvider &serviceProvider_;
        platform::espressif::EspIdfSettingsFetcher settingsFetcher_;
        platform::espressif::EspIdfSettingsParser settingsParser_;
        platform::espressif::EspIdfNvsSettingsProvider settingsProvider_;
        settings::SettingsGateImpl settingsGate_;
        settings::SettingsManager settingsManager_;
        core::WiFiManager wifiManager_;
    };
} // namespace iotsmartsys::core
