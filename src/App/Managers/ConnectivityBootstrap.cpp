#include "App/Managers/ConnectivityBootstrap.h"

#include "Version/VersionInfo.h"
#include "Config/BuildConfig.h"

namespace iotsmartsys::app
{
    ConnectivityBootstrap::ConnectivityBootstrap(core::ILogger &logger,
                                                 core::ServiceManager &serviceManager,
                                                 core::settings::SettingsManager &settingsManager,
                                                 core::WiFiManager &wifi)
        : logger_(logger),
          serviceManager_(serviceManager),
          settingsManager_(settingsManager),
          wifi_(wifi)
    {
    }

    ConnectivityBootstrap::BootPath ConnectivityBootstrap::run(core::settings::Settings &outSettings)
    {
        const auto cacheErr = settingsManager_.init();

        if (cacheErr != iotsmartsys::core::common::StateResult::Ok)
        {
            //  logger_.warn("[SettingsManager] No cached settings found (or load failed). Entering provisioning mode.");
            return BootPath::Provisioning;
        }

        if (!settingsManager_.copyCurrent(outSettings))
        {
            return BootPath::Provisioning;
        }

        logSettingsSummary(outSettings);

        serviceManager_.setLogLevel(outSettings.logLevel);

        if (outSettings.isValidWifiConfig() && !outSettings.in_config_mode && outSettings.isValidApiConfig())
        {
            iotsmartsys::core::WiFiConfig cfg;
            cfg.loadFromSettings(outSettings);
            wifi_.begin(cfg);
            return BootPath::Wifi;
        }

        return BootPath::Provisioning;
    }

    void ConnectivityBootstrap::logSettingsSummary(const core::settings::Settings &settings) const
    {
        //  logger_.info("---------------------------------------------------------");
        //  logger_.info("[SettingsManager]", " Firmware Update Mode: %s", settings.firmware.update.c_str());
        //  logger_.info("[SettingsManager]", " OTA URL: %s", settings.firmware.url.c_str());
        //  logger_.info("[SettingsManager]", " OTA Version: %s", getBuildIdentifier());
        //  logger_.info("[SettingsManager]", "Library Version: %s", IOTSMARTSYSCORE_VERSION);
        //  logger_.info("[SettingsManager]", "Log Level: %s", settings.logLevelStr());
        //  logger_.info("[SettingsManager]", "WiFi SSID: %s", settings.wifi.ssid.c_str());
#if IOTSMARTSYS_REDACT_SECRETS
        //  logger_.info("[SettingsManager]", "WiFi Password: <redacted>");
        //  logger_.info("[SettingsManager]", "API Key: <redacted>");
        //  logger_.info("[SettingsManager]", "Api Basic Auth: <redacted>");
#else
        //  logger_.info("[SettingsManager]", "WiFi Password: %s", settings.wifi.password.c_str());
        //  logger_.info("[SettingsManager]", "API Key: %s", settings.api.key.c_str());
        //  logger_.info("[SettingsManager]", "Api Basic Auth: %s", settings.api.basic_auth.c_str());
#endif
        //  logger_.info("[SettingsManager]", "API URL: %s", settings.api.url.c_str());
        //  logger_.info("[SettingsManager]", "In Config Mode: %s", settings.in_config_mode ? "Yes" : "No");

        //  logger_.info("----------------------------------------------------------");
    }
} // namespace iotsmartsys::app
