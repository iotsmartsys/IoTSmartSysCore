#include "App/Managers/ProvisioningController.h"

#include "Contracts/Settings/Settings.h"
#include "Core/Providers/ServiceManager.h"

namespace iotsmartsys::app
{
    ProvisioningController::ProvisioningController(core::ILogger &logger,
                                                   core::WiFiManager &wifi,
                                                   platform::espressif::providers::DeviceIdentityProvider &deviceIdentityProvider)
        : logger_(logger),
          wifi_(wifi),
          deviceIdentityProvider_(deviceIdentityProvider)
    {
    }

    void ProvisioningController::begin()
    {
        if (isActive())
        {
            return;
        }
        setupProvisioning();
    }

    void ProvisioningController::handle()
    {
        if (!provManager_)
        {
            return;
        }
        provManager_->handle();
    }

    bool ProvisioningController::isActive() const
    {
        return inConfigMode_ && provManager_;
    }

    void ProvisioningController::setupProvisioning()
    {
        logger_.info("--------------------------------------------------------");
        logger_.info("Entering in Config Mode - Starting Provisioning Manager");
        logger_.info("--------------------------------------------------------");
        provManager_ = new core::provisioning::ProvisioningManager();

#if defined(BLE_PROVISIONING_CHANNEL_ENABLE) && (BLE_PROVISIONING_CHANNEL_ENABLE != 0)
        bleChannel_ = new core::provisioning::BleProvisioningChannel(logger_, wifi_);
        provManager_->registerChannel(*bleChannel_);
#endif

#if defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)
        webPortalChannel_ = new core::provisioning::WebPortalProvisioningChannel(wifi_, logger_, deviceIdentityProvider_);
        provManager_->registerChannel(*webPortalChannel_);
#endif

        provManager_->onProvisioningCompleted([this](const iotsmartsys::core::provisioning::DeviceConfig &cfg)
                                              {
                                                  auto &sp_ = iotsmartsys::core::ServiceManager::instance();
                                                  auto &logger = sp_.logger();
                                                  logger.info("Provisioning completed callback invoked.");

                                                  iotsmartsys::core::settings::Settings newSettings;

                                                  newSettings.in_config_mode = false;
                                                  newSettings.wifi.ssid = cfg.wifi.ssid ? cfg.wifi.ssid : "";
                                                  newSettings.wifi.password = cfg.wifi.password ? cfg.wifi.password : "";
                                                  newSettings.api.url = cfg.deviceApiUrl ? cfg.deviceApiUrl : "";
                                                  newSettings.api.key = cfg.deviceApiKey ? cfg.deviceApiKey : "";
                                                  newSettings.api.basic_auth = cfg.basicAuth ? cfg.basicAuth : "";
                                                  sp_.settingsManager().save(newSettings);
                                                  provManager_->scheduleRestart(kProvisioningRestartDelayMs);
                                                  logger.info("Provisioning saved. Scheduling restart in 3s."); });

        provManager_->begin();
        inConfigMode_ = true;
    }
} // namespace iotsmartsys::app
