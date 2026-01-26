#include "App/Managers/ProvisioningController.h"

#include "Contracts/Settings/Settings.h"
#include "Core/Providers/ServiceManager.h"
#if IOTSMARTSYS_PROVISIONING_ENABLED && defined(BLE_PROVISIONING_CHANNEL_ENABLE) && (BLE_PROVISIONING_CHANNEL_ENABLE != 0)
#include "Platform/Espressif/Provisioning/BleProvisioningChannel.h"
#endif
#if IOTSMARTSYS_PROVISIONING_ENABLED && defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)
#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h"
#endif

namespace iotsmartsys::app
{
#if IOTSMARTSYS_PROVISIONING_ENABLED
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
#if IOTSMARTSYS_PROVISIONING_ENABLED
        if (!provisioningTaskRunning_)
        {
            BaseType_t ok = xTaskCreate(&ProvisioningController::provisioningTaskEntry,
                                        "provisioning",
                                        4096,
                                        this,
                                        4,
                                        &provisioningTask_);
            if (ok == pdPASS)
            {
                provisioningTaskRunning_ = true;
            }
            else
            {
                provisioningTask_ = nullptr;
                provisioningTaskRunning_ = false;
                logger_.warn("Provisioning task creation failed; falling back to main loop handling.");
            }
        }
#endif
    }

    void ProvisioningController::handle()
    {
        if (!provManager_)
        {
            return;
        }
#if IOTSMARTSYS_PROVISIONING_ENABLED
        if (provisioningTaskRunning_)
        {
            return;
        }
#endif
        provManager_->handle();
    }

    bool ProvisioningController::isActive() const
    {
        return inConfigMode_ && provManager_;
    }

    void ProvisioningController::setupProvisioning()
    {
       //  logger_.info("--------------------------------------------------------");
       //  logger_.info("Entering in Config Mode - Starting Provisioning Manager");
       //  logger_.info("--------------------------------------------------------");
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

#if IOTSMARTSYS_PROVISIONING_ENABLED
    void ProvisioningController::provisioningTaskEntry(void *arg)
    {
        auto *self = static_cast<ProvisioningController *>(arg);
        if (self)
        {
            self->provisioningTaskLoop();
        }
        vTaskDelete(nullptr);
    }

    void ProvisioningController::provisioningTaskLoop()
    {
        while (inConfigMode_ && provManager_)
        {
            provManager_->handle();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        provisioningTaskRunning_ = false;
        provisioningTask_ = nullptr;
    }
#endif
#else
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
    }

    void ProvisioningController::handle()
    {
    }

    bool ProvisioningController::isActive() const
    {
        return false;
    }

    void ProvisioningController::setupProvisioning()
    {
    }
#endif
} // namespace iotsmartsys::app
