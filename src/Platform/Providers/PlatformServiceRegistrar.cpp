#include "Contracts/Providers/IPlatformServiceRegistrar.h"

#if defined(ESP32)
#include "Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.h"
#endif

namespace iotsmartsys::platform
{
    IPlatformServiceRegistrar *getPlatformServiceRegistrar()
    {
#if defined(ESP32)
        return &iotsmartsys::platform::espressif::EspressifPlatformServiceRegistrar::instance();
#else
        return nullptr;
#endif
    }
} // namespace iotsmartsys::platform
