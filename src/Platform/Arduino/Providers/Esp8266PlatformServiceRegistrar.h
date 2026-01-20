#pragma once

#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Providers/IPlatformServiceRegistrar.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Platform/Arduino/Providers/ArduinoTimeProvider.h"
#include "Platform/Arduino/Providers/Esp8266RandomProvider.h"
#include "Platform/Arduino/System/Esp8266SystemControl.h"

namespace iotsmartsys::platform::arduino
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
        platform::arduino::Esp8266RandomProvider randomProvider_;
        platform::arduino::Esp8266SystemControl systemControl_;
        iotsmartsys::core::ServiceProvider &serviceProvider_;
        core::WiFiManager wifiManager_;
    };
} // namespace iotsmartsys::platform::arduino
