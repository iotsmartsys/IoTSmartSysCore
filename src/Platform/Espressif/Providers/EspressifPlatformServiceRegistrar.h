#pragma once

#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Providers/IPlatformServiceRegistrar.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Settings/SettingsManager.h"
#include "Core/Settings/SettingsGateImpl.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Platform/Espressif/Firmware/EspIdfFirmwareRuntimeInfo.h"
#include "Platform/Espressif/Providers/EspressifRandomProvider.h"
#include "Platform/Espressif/System/EspressifSystemControl.h"
#include "Platform/Espressif/Settings/EspIdfSettingsFetcher.h"
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
        platform::espressif::EspIdfFirmwareRuntimeInfo firmwareRuntimeInfo_;
        platform::espressif::EspressifRandomProvider randomProvider_;
        platform::espressif::EspressifSystemControl systemControl_;
        iotsmartsys::core::ServiceProvider &serviceProvider_;
        platform::espressif::EspIdfSettingsFetcher settingsFetcher_;
        platform::espressif::EspIdfSettingsParser settingsParser_;
        platform::espressif::EspIdfNvsSettingsProvider settingsProvider_;
        core::settings::SettingsGateImpl settingsGate_;
        core::settings::SettingsManager settingsManager_;
        core::WiFiManager wifiManager_;
    };
} // namespace iotsmartsys::platform::espressif
