#if defined(ESP32)
#include "Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.h"

#include <Arduino.h>

namespace iotsmartsys::platform::espressif
{
    EspressifPlatformServiceRegistrar &EspressifPlatformServiceRegistrar::instance()
    {
        static EspressifPlatformServiceRegistrar inst;
        return inst;
    }

    EspressifPlatformServiceRegistrar::EspressifPlatformServiceRegistrar()
        : logger_(Serial),
          timeProvider_(),
          serviceProvider_(iotsmartsys::core::ServiceProvider::init(&logger_)),
          settingsFetcher_(logger_),
          settingsParser_(),
          settingsProvider_(),
          settingsGate_(),
          settingsManager_(settingsProvider_, settingsFetcher_, settingsParser_, settingsGate_),
          wifiManager_(logger_)
    {
#if defined(SERIAL_ENABLED) && SERIAL_ENABLED == 1
#ifdef SERIAL_BAUD_RATE
        Serial.begin(SERIAL_BAUD_RATE);
#else
        Serial.begin(115200);
#endif
#endif
    }

    void EspressifPlatformServiceRegistrar::registerPlatformServices(iotsmartsys::core::ServiceProvider &sp)
    {
        sp.setLogger(&logger_);
        sp.setTime(&timeProvider_);
        sp.setSettings(&settingsManager_);
        sp.setSettingsGate(&settingsGate_);
        sp.setSettingsManager(&settingsManager_);
        sp.setWiFiManager(&wifiManager_);
    }
} // namespace iotsmartsys::platform::espressif
#endif
