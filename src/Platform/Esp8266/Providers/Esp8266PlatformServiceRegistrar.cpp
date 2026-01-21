#if defined(ESP8266)
#include "Platform/Esp8266/Providers/Esp8266PlatformServiceRegistrar.h"

#include <Arduino.h>

namespace iotsmartsys::platform::esp8266
{
    Esp8266PlatformServiceRegistrar &Esp8266PlatformServiceRegistrar::instance()
    {
        static Esp8266PlatformServiceRegistrar inst;
        return inst;
    }

    Esp8266PlatformServiceRegistrar::Esp8266PlatformServiceRegistrar()
        : logger_(Serial),
          timeProvider_(),
          serviceProvider_(iotsmartsys::core::ServiceProvider::init(&logger_)),
          settingsFetcher_(logger_),
          settingsProvider_(),
          settingsParser_(),
          settingsGate_(),
          settingsManager_(settingsProvider_, settingsFetcher_, settingsParser_, settingsGate_),
          wifiManager_(logger_)
    {
        Serial.begin(115200);
    }

    void Esp8266PlatformServiceRegistrar::registerPlatformServices(iotsmartsys::core::ServiceProvider &sp)
    {
        sp.setLogger(&logger_);
        sp.setTime(&timeProvider_);
        sp.setSettings(&settingsManager_);
        sp.setSettingsGate(&settingsGate_);
        sp.setSettingsManager(&settingsManager_);
        sp.setWiFiManager(&wifiManager_);
    }
} // namespace iotsmartsys::platform::esp8266
#endif
