#pragma once

namespace iotsmartsys::core
{
    class ServiceProvider;
}

namespace iotsmartsys::platform
{
    class IPlatformServiceRegistrar
    {
    public:
        virtual ~IPlatformServiceRegistrar() = default;
        virtual void registerPlatformServices(iotsmartsys::core::ServiceProvider &sp) = 0;
    };

    IPlatformServiceRegistrar *getPlatformServiceRegistrar();
} // namespace iotsmartsys::platform
