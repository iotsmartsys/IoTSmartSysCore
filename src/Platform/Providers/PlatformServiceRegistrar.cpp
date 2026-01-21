#include "Contracts/Providers/IPlatformServiceRegistrar.h"

#if defined(ESP32)
#include "Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.h"
#elif defined(ESP8266)
#include "Platform/Esp8266/Providers/Esp8266PlatformServiceRegistrar.h"
#endif

namespace iotsmartsys::platform
{
    IPlatformServiceRegistrar *getPlatformServiceRegistrar()
    {
#if defined(ESP32)
        return &iotsmartsys::platform::espressif::EspressifPlatformServiceRegistrar::instance();
#elif defined(ESP8266)
    return &iotsmartsys::platform::esp8266::Esp8266PlatformServiceRegistrar::instance();
#else
        return nullptr;
#endif
    }
} // namespace iotsmartsys::platform
