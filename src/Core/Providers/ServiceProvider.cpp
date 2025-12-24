#include "Contracts/Providers/ServiceProvider.h"

namespace iotsmartsys::core
{

    ServiceProvider &ServiceProvider::instance()
    {
        static ServiceProvider inst;
        return inst;
    }
    ServiceProvider &ServiceProvider::init()
    {
        return instance();
    }
    ServiceProvider &ServiceProvider::init(ILogger *logger)
    {
        instance().setLogger(logger);
        return instance();
    }

    void ServiceProvider::setLogger(ILogger *logger) { _logger = logger; }
    void ServiceProvider::setTime(ITimeProvider *time) { _time = time; }

    void ServiceProvider::setSettings(settings::IReadOnlySettingsProvider *settings) { _settingsProvider = settings; }
    void ServiceProvider::setSettingsGate(settings::ISettingsGate *gate) { _settingsGate = gate; }
    void ServiceProvider::setSettingsManager(settings::SettingsManager *manager) { _settingsManager = manager; }

    ILogger *ServiceProvider::logger() const { return _logger; }
    ITimeProvider *ServiceProvider::time() const { return _time; }

    settings::IReadOnlySettingsProvider *ServiceProvider::getSettingsProvider() const { return _settingsProvider; }
    settings::ISettingsGate *ServiceProvider::getSettingsGate() const { return _settingsGate; }
    settings::SettingsManager *ServiceProvider::getSettingsManager() const { return _settingsManager; }

    bool ServiceProvider::isReady() const
    {
        return _logger && _time && _settingsProvider && _settingsGate && _settingsManager;
    }

} // namespace iotsmartsys::core