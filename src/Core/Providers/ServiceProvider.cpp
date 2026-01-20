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
    void ServiceProvider::setWiFiManager(core::WiFiManager *wifi) { _wifiManager = wifi; }
    void ServiceProvider::setFirmwareRuntimeInfo(firmware::IFirmwareRuntimeInfo *info) { _firmwareRuntimeInfo = info; }
    void ServiceProvider::setRandomProvider(IRandomProvider *random) { _randomProvider = random; }
    void ServiceProvider::setSystemControl(ISystemControl *systemControl) { _systemControl = systemControl; }

    ILogger *ServiceProvider::logger() const { return _logger; }
    ITimeProvider *ServiceProvider::time() const { return _time; }

    settings::IReadOnlySettingsProvider *ServiceProvider::getSettingsProvider() const { return _settingsProvider; }
    settings::ISettingsGate *ServiceProvider::getSettingsGate() const { return _settingsGate; }
    settings::SettingsManager *ServiceProvider::getSettingsManager() const { return _settingsManager; }
    core::WiFiManager *ServiceProvider::getWiFiManager() const { return _wifiManager; }
    firmware::IFirmwareRuntimeInfo *ServiceProvider::getFirmwareRuntimeInfo() const { return _firmwareRuntimeInfo; }
    IRandomProvider *ServiceProvider::getRandomProvider() const { return _randomProvider; }
    ISystemControl *ServiceProvider::getSystemControl() const { return _systemControl; }

    bool ServiceProvider::isReady() const
    {
        return _logger && _time && _settingsProvider && _settingsGate && _settingsManager && _wifiManager;
    }

} // namespace iotsmartsys::core
