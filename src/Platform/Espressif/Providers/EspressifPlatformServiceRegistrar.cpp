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
          wifiManager_(logger_),
          binaryCapabilityStateProvider_()
    {
#if defined(SERIAL_ENABLED) && SERIAL_ENABLED == 1
#ifdef SERIAL_BAUD_RATE
        Serial.begin(SERIAL_BAUD_RATE);
#else
        Serial.begin(115200);
#endif
#endif
    }

    namespace
    {
        std::uint32_t g_registrations = 0;
        std::uint32_t g_snapshotLoads = 0;
    } // namespace

    std::uint32_t EspressifPlatformServiceRegistrar::registrationCount() { return g_registrations; }
    std::uint32_t EspressifPlatformServiceRegistrar::snapshotLoadCount() { return g_snapshotLoads; }

    void EspressifPlatformServiceRegistrar::registerPlatformServices(iotsmartsys::core::ServiceProvider &sp)
    {
        ++g_registrations;
        sp.setLogger(&logger_);
        sp.setTime(&timeProvider_);
        sp.setSettings(&settingsManager_);
        sp.setSettingsGate(&settingsGate_);
        sp.setSettingsManager(&settingsManager_);
        sp.setWiFiManager(&wifiManager_);
        sp.setBinaryCapabilityStateProvider(&binaryCapabilityStateProvider_);
        // BCS-007: single NVS data read for the boot, before any capability exists.
        ++g_snapshotLoads;
        binaryCapabilityStateProvider_.loadSnapshot();
    }
} // namespace iotsmartsys::platform::espressif
#endif
