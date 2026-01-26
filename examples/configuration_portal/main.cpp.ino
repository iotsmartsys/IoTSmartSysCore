#include <WiFi.h>
#include "Core/Provisioning/ProvisioningManager.h"
#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h"
// #include "Platform/Arduino/Provisioning/BleProvisioningChannel.h"
#include "Core/Providers/ServiceManager.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "Contracts/Connections/WiFiManager.h"

using namespace iotsmartsys::core::provisioning;
using namespace iotsmartsys::core;

static iotsmartsys::platform::arduino::ArduinoSerialLogger logger_(Serial);
static iotsmartsys::core::WiFiManager wifi_(logger_);

ProvisioningManager provManager;
WebPortalProvisioningChannel portalChannel(wifi_, logger_);
auto &sp_ = iotsmartsys::core::ServiceManager::init();

void setup()
{
    sp_.setLogLevel(iotsmartsys::core::LogLevel::Debug);
    provManager.registerChannel(portalChannel);
    // provManager.registerChannel(bleChannel);
    provManager.onProvisioningCompleted([](const DeviceConfig &cfg)
                                        {

           //  logger_.info("Provisioning completed callback invoked.");
            
            auto &sm = sp_.settingsManager();

                iotsmartsys::core::settings::Settings newSettings;
                newSettings.wifi.ssid = cfg.wifi.ssid ? cfg.wifi.ssid : "";
                newSettings.wifi.password = cfg.wifi.password ? cfg.wifi.password : "";
                newSettings.api.key = cfg.deviceApiKey ? cfg.deviceApiKey : "";
                newSettings.api.basic_auth = cfg.basicAuth ? cfg.basicAuth : "";
               //  logger_.info("Saving new settings via SettingsManager.");
                sm.save(newSettings); });

    auto &settingsManager_ = sp_.settingsManager();
    const auto cacheErr = settingsManager_.init();
   //  logger_.info("SettingsManager init() completed with result %d", (int)cacheErr);

    iotsmartsys::core::settings::Settings settings_;
    if (cacheErr == iotsmartsys::core::common::StateResult::Ok)
    {
        if (settingsManager_.copyCurrent(settings_))
        {
           //  logger_.info("[SettingsManager] Loaded settings from NVS cache.");
           //  logger_.warn("WiFi SSID='%s'", settings_.wifi.ssid.c_str());
           //  logger_.warn("WiFi Password='%s'", settings_.wifi.password.c_str());
           //  logger_.warn("API Key='%s'", settings_.api.key.c_str());
           //  logger_.warn("API Basic Auth='%s'", settings_.api.basic_auth.c_str());
           //  logger_.warn("MQtt Broker Host='%s'", settings_.mqtt.primary.host.c_str());
           //  logger_.warn("MQtt Broker Port=%d", settings_.mqtt.primary.port);
           //  logger_.warn("MQtt Broker User='%s'", settings_.mqtt.primary.user.c_str());
           //  logger_.warn("MQtt Broker Password='%s'", settings_.mqtt.primary.password.c_str());
           //  logger_.warn("MQtt Broker TTL=%d", settings_.mqtt.primary.ttl);
           //  logger_.warn("NIVEL DE LOG ATUAL: %s", settings_.logLevelStr());
        }
        else
        {
           //  logger_.warn("[SettingsManager] Failed to copy current settings from NVS.");
        }
    }
    else
    {
       //  logger_.warn("[SettingsManager] No cached settings found (or load failed). Using compile-time defaults until API refresh.");
    }

    provManager.begin();
}

void loop()
{
    provManager.handle();
    // resto do app...
}
