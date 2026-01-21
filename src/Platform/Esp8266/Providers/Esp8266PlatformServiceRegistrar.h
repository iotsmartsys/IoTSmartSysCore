#pragma once

#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Providers/IPlatformServiceRegistrar.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Settings/SettingsManager.h"
#include "Core/Settings/SettingsGateImpl.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Platform/Esp8266/Settings/Esp8266SettingsFetcher.h"
#include "Platform/Esp8266/Settings/Providers/Esp8266NvsSettingsProvider.h"
#include "Platform/Espressif/Settings/EspIdfSettingsParser.h"

namespace iotsmartsys::platform::esp8266
{
    class Esp8266PlatformServiceRegistrar final : public iotsmartsys::platform::IPlatformServiceRegistrar
    {
    public:
        static Esp8266PlatformServiceRegistrar &instance();

        void registerPlatformServices(iotsmartsys::core::ServiceProvider &sp) override;

    private:
        Esp8266PlatformServiceRegistrar();

        platform::arduino::ArduinoSerialLogger logger_;
        platform::arduino::ArduinoTimeProvider timeProvider_;
        iotsmartsys::core::ServiceProvider &serviceProvider_;
    platform::esp8266::Esp8266SettingsFetcher settingsFetcher_;
    platform::esp8266::Esp8266NvsSettingsProvider settingsProvider_;
    platform::espressif::EspIdfSettingsParser settingsParser_;
        core::settings::SettingsGateImpl settingsGate_;
        core::settings::SettingsManager settingsManager_;
        core::WiFiManager wifiManager_;
    };
} // namespace iotsmartsys::platform::esp8266
