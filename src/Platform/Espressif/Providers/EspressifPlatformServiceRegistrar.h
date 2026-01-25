#pragma once

#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Providers/IPlatformServiceRegistrar.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Settings/SettingsManager.h"
#include "Config/BuildConfig.h"
#include "Core/Settings/SettingsGateImpl.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#if IOTSMARTSYS_SETTINGS_FETCH_ENABLED
#include "Platform/Espressif/Settings/EspIdfSettingsFetcher.h"
#else
#include "Platform/Espressif/Settings/NullSettingsFetcher.h"
#endif
#include "Platform/Espressif/Settings/EspIdfSettingsParser.h"
#include "Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.h"

namespace iotsmartsys::platform::espressif
{
    class EspressifPlatformServiceRegistrar final : public iotsmartsys::platform::IPlatformServiceRegistrar
    {
    public:
        static EspressifPlatformServiceRegistrar &instance();

        void registerPlatformServices(iotsmartsys::core::ServiceProvider &sp) override;

    private:
        EspressifPlatformServiceRegistrar();

        platform::arduino::ArduinoSerialLogger logger_;
        platform::arduino::ArduinoTimeProvider timeProvider_;
        iotsmartsys::core::ServiceProvider &serviceProvider_;
#if IOTSMARTSYS_SETTINGS_FETCH_ENABLED
        platform::espressif::EspIdfSettingsFetcher settingsFetcher_;
#else
        platform::espressif::NullSettingsFetcher settingsFetcher_;
#endif
        platform::espressif::EspIdfSettingsParser settingsParser_;
        platform::espressif::EspIdfNvsSettingsProvider settingsProvider_;
        core::settings::SettingsGateImpl settingsGate_;
        core::settings::SettingsManager settingsManager_;
        core::WiFiManager wifiManager_;
    };
} // namespace iotsmartsys::platform::espressif
