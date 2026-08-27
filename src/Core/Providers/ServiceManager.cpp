#include "Core/Providers/ServiceManager.h"
#include "Contracts/Providers/IPlatformServiceRegistrar.h"

#include <new>

namespace iotsmartsys::core
{
    namespace
    {
        // BCS-024: the build uses -fno-threadsafe-statics, so a function-local
        // static cannot be relied upon for a single, race-free construction.
        // A namespace-scope buffer plus an explicit pointer makes both public
        // accessors converge on one object, constructed exactly once during the
        // bootstrap and before any task/callback can reach it.
        alignas(ServiceManager) unsigned char g_serviceManagerStorage[sizeof(ServiceManager)];
        ServiceManager *g_serviceManager = nullptr;
    } // namespace

    ServiceManager &ServiceManager::init()
    {
        if (!g_serviceManager)
        {
            g_serviceManager = new (g_serviceManagerStorage) ServiceManager();
        }
        return *g_serviceManager;
    }

    ServiceManager &ServiceManager::instance()
    {
        // Resolves the very same object as init(). The bootstrap completes
        // init() before concurrent access, so this never builds a second graph
        // nor re-registers platform services / re-reads the binary snapshot.
        return init();
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
        if (auto *console = serviceProvider_.screenConsole())
        {
            Screen::setConsole(console);
        }
        if (auto *time = serviceProvider_.time())
        {
            Time::setProvider(time);
        }
    }

    ILogger &ServiceManager::logger()
    {
        return *serviceProvider_.logger();
    }
    IScreenConsole &ServiceManager::screenConsole() { return Screen::get(); }
    ITimeProvider &ServiceManager::timeProvider() { return *serviceProvider_.time(); }
    settings::SettingsManager &ServiceManager::settingsManager() { return *serviceProvider_.getSettingsManager(); }
    settings::ISettingsGate &ServiceManager::settingsGate() { return *serviceProvider_.getSettingsGate(); }
    settings::IReadOnlySettingsProvider &ServiceManager::settingsProvider() { return *serviceProvider_.getSettingsProvider(); }
    core::WiFiManager &ServiceManager::wifiManager() { return *serviceProvider_.getWiFiManager(); }

    providers::IBinaryCapabilityStateProvider *ServiceManager::binaryCapabilityStateProvider()
    {
        return serviceProvider_.getBinaryCapabilityStateProvider();
    }

    common::StateResult ServiceManager::activateBinaryStateWriter()
    {
        auto *provider = serviceProvider_.getBinaryCapabilityStateProvider();
        if (!provider)
        {
            return common::StateResult::NotSupported;
        }
        return provider->activateWriter();
    }

    void ServiceManager::setLogLevel(LogLevel level) { logger().setMinLevel(level); }
} // namespace iotsmartsys::core
