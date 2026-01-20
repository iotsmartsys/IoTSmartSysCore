#include "Core/Providers/ServiceManager.h"
#include "Contracts/Providers/IPlatformServiceRegistrar.h"

namespace iotsmartsys::core
{
    ServiceManager &ServiceManager::init()
    {
        static ServiceManager instance;
        return instance;
    }

    ServiceManager &ServiceManager::instance()
    {
        static ServiceManager instance;
        return instance;
    }

    ServiceManager::ServiceManager()
        : serviceProvider_(ServiceProvider::init())
    {
        if (auto *registrar = platform::getPlatformServiceRegistrar())
        {
            registrar->registerPlatformServices(serviceProvider_);
        }
        registerServices();
    }

    void ServiceManager::registerServices()
    {
        if (auto *logger = serviceProvider_.logger())
        {
            Log::setLogger(logger);
        }
        if (auto *time = serviceProvider_.time())
        {
            Time::setProvider(time);
        }
    }

    ILogger &ServiceManager::logger() { return *serviceProvider_.logger(); }
    ITimeProvider &ServiceManager::timeProvider() { return *serviceProvider_.time(); }
    settings::SettingsManager &ServiceManager::settingsManager() { return *serviceProvider_.getSettingsManager(); }
    settings::ISettingsGate &ServiceManager::settingsGate() { return *serviceProvider_.getSettingsGate(); }
    settings::IReadOnlySettingsProvider &ServiceManager::settingsProvider() { return *serviceProvider_.getSettingsProvider(); }
    core::WiFiManager &ServiceManager::wifiManager() { return *serviceProvider_.getWiFiManager(); }
    firmware::IFirmwareRuntimeInfo *ServiceManager::firmwareRuntimeInfo() { return serviceProvider_.getFirmwareRuntimeInfo(); }

    void ServiceManager::setLogLevel(LogLevel level) { logger().setMinLevel(level); }
} // namespace iotsmartsys::core
