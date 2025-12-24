#include "Core/Providers/ServiceManager.h"

namespace iotsmartsys::core
{
    ServiceManager &ServiceManager::init()
    {
        static ServiceManager instance;
        return instance;
    }

    ServiceManager::ServiceManager()
        : logger_(Serial),
          timeProvider_(),
          serviceProvider_(ServiceProvider::init(&logger_)),
          settingsFetcher_(),
          settingsParser_(),
          settingsProvider_(),
          settingsGate_(),
          settingsManager_(settingsProvider_, settingsFetcher_, settingsParser_, settingsGate_),
          wifiManager_(logger_)
    {
        registerServices();
    }

    void ServiceManager::registerServices()
    {
        Log::setLogger(&logger_);
        Time::setProvider(&timeProvider_);

        serviceProvider_.setLogger(&logger_);
        serviceProvider_.setTime(&Time::get());
        serviceProvider_.setSettings(&settingsManager_);
        serviceProvider_.setSettingsGate(&settingsGate_);
        serviceProvider_.setSettingsManager(&settingsManager_);
        serviceProvider_.setWiFiManager(&wifiManager_);
    }

    ILogger &ServiceManager::logger() { return logger_; }
    ITimeProvider &ServiceManager::timeProvider() { return timeProvider_; }
    settings::SettingsManager &ServiceManager::settingsManager() { return settingsManager_; }
    settings::ISettingsGate &ServiceManager::settingsGate() { return settingsGate_; }
    settings::IReadOnlySettingsProvider &ServiceManager::settingsProvider() { return settingsManager_; }
    core::WiFiManager &ServiceManager::wifiManager() { return wifiManager_; }

    void ServiceManager::setLogLevel(LogLevel level) { logger_.setMinLevel(level); }
} // namespace iotsmartsys::core
