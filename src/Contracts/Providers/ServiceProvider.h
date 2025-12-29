#pragma once

#include "IServiceProvider.h"

namespace iotsmartsys::core
{

    class ServiceProvider final : public IServiceProvider
    {
    public:
        static ServiceProvider &instance();
        static ServiceProvider &init();
        static ServiceProvider &init(ILogger *logger);

        // Registro (bootstrap)
        void setLogger(ILogger *logger);
        void setTime(ITimeProvider *time);

        void setSettings(settings::IReadOnlySettingsProvider *settings);
        void setSettingsGate(settings::ISettingsGate *gate);
        void setSettingsManager(settings::SettingsManager *manager);
        void setWiFiManager(core::WiFiManager *wifi);

        // IServiceProvider
        ILogger *logger() const override;
        ITimeProvider *time() const override;

        settings::IReadOnlySettingsProvider *getSettingsProvider() const override;
        settings::ISettingsGate *getSettingsGate() const override;

        settings::SettingsManager *getSettingsManager() const override;
        core::WiFiManager *getWiFiManager() const override;

        // útil para debug (opcional)
        bool isReady() const;

    private:
        ServiceProvider() = default;

    private:
        ILogger *_logger{nullptr};
        ITimeProvider *_time{nullptr};

        settings::IReadOnlySettingsProvider *_settingsProvider{nullptr};
        settings::ISettingsGate *_settingsGate{nullptr};
        settings::SettingsManager *_settingsManager{nullptr};
        core::WiFiManager *_wifiManager{nullptr};
    };

} // namespace iotsmartsys::core