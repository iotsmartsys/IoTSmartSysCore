#include "SmartSysApp.h"

#include <cstdio>

namespace iotsmartsys
{
    SmartSysApp::SmartSysApp()
        : serviceManager_(core::ServiceManager::init()),
          logger_(serviceManager_.logger()),
          settingsManager_(serviceManager_.settingsManager()),
          settingsGate_(serviceManager_.settingsGate()),
          mqttClient_(logger_),
          mqttSink_(mqttClient_),
          builder_(hwFactory_,
                   mqttSink_,
                   capSlots_,
                   capDtors_,
                   8,
                   adapterSlots_,
                   adapterDtors_,
                   8,
                   arena_,
                   sizeof(arena_)),
          wifi_(logger_),
          mqtt_(mqttClient_, logger_, settingsGate_, settingsManager_)
    {
    }

    void SmartSysApp::applySettingsToRuntime(const iotsmartsys::core::settings::Settings &)
    {
        return;
    }

    void SmartSysApp::onMqttConnected()
    {
        logger_.info("MQTT connected.");
    }

    void SmartSysApp::onMqttMessage(const core::MqttMessageView &msg)
    {
        logger_.info("=== MQTT Message Received ===");

        iotsmartsys::core::DeviceCommand cmd;
        commandParser_.parseCommand(msg.payload, msg.payloadLen, cmd);
        logger_.info("Capability: %s", cmd.capability_name);
        logger_.info("Value: %s", cmd.value);
        logger_.info("device_id: %s", cmd.device_id);
        logger_.info("=== End MQTT Message ===");
        if (capabilityManager_)
        {
            auto *cap = capabilityManager_->getCommandCapabilityByName(cmd.capability_name);
            if (cap)
            {
                logger_.info("Found capability: %s", cmd.capability_name);
                iotsmartsys::core::CapabilityCommand capabilityCmd;
                capabilityCmd.capability_name = cmd.capability_name;
                capabilityCmd.value = cmd.value;
                cap->applyCommand(capabilityCmd);
            }
            else
            {
                logger_.error("Capability not found: %s", cmd.capability_name);
            }
        }
        else
        {
            logger_.error("CapabilityManager is null. Cannot apply command.");
        }
    }

    void SmartSysApp::onSettingsUpdated(const iotsmartsys::core::settings::Settings &newSettings)
    {
        logger_.warn("[SettingsManager] Settings updated from API. Re-applying runtime config...");
        if (newSettings.hasChanges())
        {
            logger_.warn("[SettingsManager] Settings have changes. Applying...");
        }
        else
        {
            logger_.warn("[SettingsManager] Settings have NO changes. Skipping apply.");
            return;
        }
        applySettingsToRuntime(newSettings);
    }

    void SmartSysApp::setup()
    {
        Serial.begin(115200);
        delay(5000);

        serviceManager_.setLogLevel(core::LogLevel::Debug);
        core::Log::setLogger(&logger_);

        logger_.info("Starting IoT SmartSys Core example...");

        settingsManager_.setUpdatedCallback(SmartSysApp::onSettingsUpdatedThunk, this);
        logger_.info("SettingsManager callback for updates set.");

        const auto cacheErr = settingsManager_.init();
        logger_.info("SettingsManager init() completed with result %d", (int)cacheErr);

        if (cacheErr == iotsmartsys::core::common::StateResult::Ok)
        {
            if (settingsManager_.copyCurrent(settings_))
            {
                serviceManager_.setLogLevel(settings_.logLevel);
                logger_.info("[SettingsManager] Loaded settings from NVS cache.");
                logger_.error("In Config Mode=%s", settings_.in_config_mode ? "true" : "false");
                logger_.error("WiFi SSID='%s'", settings_.wifi.ssid.c_str());
                logger_.error("WiFi Password='%s'", settings_.wifi.password.c_str());
                logger_.error("MQtt Broker Host='%s'", settings_.mqtt.primary.host.c_str());
                logger_.error("MQtt Broker Port=%d", settings_.mqtt.primary.port);
                logger_.error("MQtt Broker User='%s'", settings_.mqtt.primary.user.c_str());
                logger_.error("MQtt Broker Password='%s'", settings_.mqtt.primary.password.c_str());
                logger_.error("MQtt Broker TTL=%d", settings_.mqtt.primary.ttl);
                logger_.error("NIVEL DE LOG ATUAL: %s", settings_.logLevelStr());
                logger_.error("API URL: %s", settings_.api.url.c_str());
                logger_.error("API Token: %s", settings_.api.key.c_str());
                logger_.error("API auth: %s", settings_.api.basic_auth.c_str());

                if (settings_.isValidWifiConfig() && !settings_.in_config_mode && settings_.isValidApiConfig())
                {
                    logger_.info("[SettingsManager] Applying cached WiFi settings from NVS.");
                    iotsmartsys::core::WiFiConfig cfg;
                    cfg.loadFromSettings(settings_);

                    wifi_.begin(cfg);
                }
                else
                {
                    logger_.warn("[SettingsManager] Cached WiFi settings are invalid. Skipping entrando no modo de configuração.");

                    setupProvisioningConfiguration();
                    return;
                }
            }
        }
        else
        {
            logger_.warn("[SettingsManager] No cached settings found (or load failed). Entering provisioning mode.");
            setupProvisioningConfiguration();
            return;
        }

        logger_.info("ClientId do Device: %s", settings_.clientId);

        iotsmartsys::core::ConnectivityGate::init(latch_);
        logger_.info("ConnectivityGate initialized.");

        char topic[128];
        snprintf(topic, sizeof(topic), "device/%s/command",
                 settings_.clientId ? settings_.clientId : "");
        logger_.info("MAIND -- snprintf to topic: %s", topic);
        mqtt_.subscribe(topic);
        mqtt_.setOnMessage(&SmartSysApp::onMqttMessageThunk, this);
        mqtt_.setOnConnected(&SmartSysApp::onMqttConnectedThunk, this);

        logger_.info("MQTT onMessage callback set.");

        static iotsmartsys::core::CapabilityManager capManager = builder_.build();
        capabilityManager_ = &capManager;
        capabilityManager_->setup();

        delay(1000);
    }

    void SmartSysApp::handle()
    {
        if (inConfigMode_ && provManager)
        {
            provManager->handle();
            return;
        }

        if (capabilityManager_)
        {
            capabilityManager_->handle();
        }

        wifi_.handle();
        mqtt_.handle();
        settingsManager_.handle();
    }

    void SmartSysApp::setupProvisioningConfiguration()
    {
        logger_.info("--------------------------------------------------------");
        logger_.info("Entering in Config Mode - Starting Provisioning Manager");
        logger_.info("--------------------------------------------------------");
        provManager = new core::provisioning::ProvisioningManager();

        bleChannel = new core::provisioning::BleProvisioningChannel(logger_, wifi_);
        provManager->registerChannel(*bleChannel);
        provManager->onProvisioningCompleted([](const iotsmartsys::core::provisioning::DeviceConfig &cfg)
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
                                                 logger.info("New WiFi SSID='%s'", newSettings.wifi.ssid.c_str());
                                                 logger.info("New WiFi Password='%s'", newSettings.wifi.password.c_str());
                                                 logger.info("New API URL='%s'", newSettings.api.url.c_str());
                                                 logger.info("New API Key='%s'", newSettings.api.key.c_str());
                                                 logger.info("New API Basic Auth='%s'", newSettings.api.basic_auth.c_str());
                                                 logger.info("Saving new settings via SettingsManager.");
                                                 sp_.settingsManager().save(newSettings);
                                                 ESP.restart(); });

        provManager->begin();
        inConfigMode_ = true;
    }

    void SmartSysApp::onMqttMessageThunk(void *ctx, const core::MqttMessageView &msg)
    {
        if (!ctx)
        {
            return;
        }
        static_cast<SmartSysApp *>(ctx)->onMqttMessage(msg);
    }

    void SmartSysApp::onMqttConnectedThunk(void *ctx)
    {
        if (!ctx)
        {
            return;
        }
        static_cast<SmartSysApp *>(ctx)->onMqttConnected();
    }

    void SmartSysApp::onSettingsUpdatedThunk(const core::settings::Settings &newSettings, void *ctx)
    {
        if (!ctx)
        {
            return;
        }
        static_cast<SmartSysApp *>(ctx)->onSettingsUpdated(newSettings);
    }

    /// @brief Adds a new alarm capability to the application.
    iotsmartsys::core::AlarmCapability *SmartSysApp::addAlarmCapability(iotsmartsys::app::AlarmConfig cfg)
    {
        return builder_.addAlarm(cfg);
    }

    /// @brief Adds a new door sensor capability to the application.
    iotsmartsys::core::DoorSensorCapability *SmartSysApp::addDoorSensorCapability(iotsmartsys::app::DoorSensorConfig cfg)
    {
        return builder_.addDoorSensor(cfg);
    }

    /// @brief Adds a new clap sensor capability to the application.
    iotsmartsys::core::ClapSensorCapability *SmartSysApp::addClapSensorCapability(iotsmartsys::app::ClapSensorConfig cfg)
    {
        return builder_.addClapSensor(cfg);
    }

    /// @brief Adds a new light capability to the application.
    iotsmartsys::core::LightCapability *SmartSysApp::addLightCapability(iotsmartsys::app::LightConfig cfg)
    {
        return builder_.addLight(cfg);
    }

    /// @brief Adds a new GLP sensor capability to the application.
    iotsmartsys::core::GlpSensorCapability *SmartSysApp::addGlpSensorCapability(iotsmartsys::app::GlpSensorConfig cfg)
    {
        return builder_.addGlpSensor(cfg);
    }

    /// @brief Adds a new GLP meter capability to the application.
    iotsmartsys::core::GlpMeterCapability *SmartSysApp::addGlpMeterCapability(iotsmartsys::app::GlpMeterConfig cfg)
    {
        return builder_.addGlpMeter(cfg);
    }

    /// @brief Adds a new humidity sensor capability to the application.
    iotsmartsys::core::HumiditySensorCapability *SmartSysApp::addHumiditySensorCapability(iotsmartsys::app::HumiditySensorConfig cfg)
    {
        return builder_.addHumiditySensor(cfg);
    }

    /// @brief Adds a new height water level capability to the application.
    iotsmartsys::core::HeightWaterLevelCapability *SmartSysApp::addHeightWaterLevelCapability(iotsmartsys::app::WaterLevelSensorConfig cfg)
    {
        return builder_.addWaterHeight(cfg);
    }

    /// @brief Adds a new LED capability to the application.
    iotsmartsys::core::LEDCapability *SmartSysApp::addLedCapability(iotsmartsys::app::LightConfig cfg)
    {
        return builder_.addLED(cfg);
    }

    /// @brief Adds a new PIR sensor capability to the application.
    iotsmartsys::core::PirSensorCapability *SmartSysApp::addPirSensorCapability(iotsmartsys::app::PirSensorConfig cfg)
    {
        return builder_.addPirSensor(cfg);
    }

    /// @brief Adds a new push button capability to the application.
    iotsmartsys::core::PushButtonCapability *SmartSysApp::addPushButtonCapability(iotsmartsys::app::PushButtonConfig cfg)
    {
        return builder_.addPushButton(cfg);
    }

    /// @brief Adds a new touch button capability to the application.
    iotsmartsys::core::TouchButtonCapability *SmartSysApp::addTouchButtonCapability(iotsmartsys::app::TouchButtonConfig cfg)
    {
        return builder_.addTouchButton(cfg);
    }

    /// @brief Adds a new switch capability to the application.
    iotsmartsys::core::SwitchCapability *SmartSysApp::addSwitchCapability(iotsmartsys::app::SwitchConfig cfg)
    {
        return builder_.addSwitch(cfg);
    }

    /// @brief Adds a new valve capability to the application.
    iotsmartsys::core::ValveCapability *SmartSysApp::addValveCapability(iotsmartsys::app::ValveConfig cfg)
    {
        return builder_.addValve(cfg);
    }

    /// @brief Adds a new operational color sensor capability to the application.
    iotsmartsys::core::OperationalColorSensorCapability *SmartSysApp::addOperationalColorSensorCapability(iotsmartsys::app::OperationalColorSensorConfig cfg)
    {
        return builder_.addOperationalColorSensor(cfg);
    }

    /// @brief Adds a new temperature sensor capability to the application.
    iotsmartsys::core::TemperatureSensorCapability *SmartSysApp::addTemperatureSensorCapability(iotsmartsys::app::TemperatureSensorConfig cfg)
    {
        return builder_.addTemperatureSensor(cfg);
    }

    /// @brief Adds a new water level liters capability to the application.
    iotsmartsys::core::WaterLevelLitersCapability *SmartSysApp::addWaterLevelLitersCapability(iotsmartsys::app::WaterLevelSensorConfig cfg)
    {
        return builder_.addWaterLevelLiters(cfg);
    }

    /// @brief Adds a new water level percent capability to the application.
    iotsmartsys::core::WaterLevelPercentCapability *SmartSysApp::addWaterLevelPercentCapability(iotsmartsys::app::WaterLevelSensorConfig cfg)
    {
        return builder_.addWaterLevelPercent(cfg);
    }

} // namespace iotsmartsys
