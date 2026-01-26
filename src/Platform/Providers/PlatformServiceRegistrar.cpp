#include "Contracts/Providers/IPlatformServiceRegistrar.h"

#include "Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.h"

namespace iotsmartsys::platform
{
    IPlatformServiceRegistrar *getPlatformServiceRegistrar()
    {
        return &iotsmartsys::platform::espressif::EspressifPlatformServiceRegistrar::instance();
    }
} // namespace iotsmartsys::platform
