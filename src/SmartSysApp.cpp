#include "SmartSysApp.h"
#include "App/Builders/Builders/AnnouncePayloadBuilder.h"
#include "Version/VersionInfo.h"
#include "Contracts/Commands/ICommandProcessor.h"
#include "Contracts/Commands/CommandTypes.h"
#include "esp_ota_ops.h"

#include <cstdio>
using namespace iotsmartsys::platform::espressif::ota;

namespace iotsmartsys
{
    SmartSysApp::SmartSysApp()
        : serviceManager_(core::ServiceManager::init()),
          logger_(serviceManager_.logger()),
          settingsManager_(serviceManager_.settingsManager()),
          settingsGate_(serviceManager_.settingsGate()),
          mqttClient_(logger_),
          mqttSink_(mqttClient_, settingsManager_),
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
          mqtt_(mqttClient_, logger_, settingsGate_, settingsManager_),
          manifestParser_(),
#ifdef OTA_ENABLED
          ota_(logger_),
#endif
          otaManager_(settingsManager_, logger_, manifestParser_,
#ifdef OTA_ENABLED
                      ota_,
#endif
                      settingsGate_),
          commandParser_(logger_),
          transportHub_(logger_, settingsManager_)
    {
    }

    void SmartSysApp::configureFactoryResetButton(int pin, bool activeLow)
    {
        if (factoryResetButton_)
        {
            hwFactory_.outputAdapterDestructor()(factoryResetButton_);
            factoryResetButton_ = nullptr;
        }
        void *mem = malloc(hwFactory_.outputAdapterSize());
        if (!mem)
        {
            logger_.error("Failed to allocate memory for factory reset button adapter.");
            return;
        }
        factoryResetButton_ = hwFactory_.createOutput(mem, static_cast<std::uint8_t>(pin), !activeLow);
        factoryResetButton_->setup();
    }

    void SmartSysApp::configureSerialTransport(HardwareSerial &serial, uint32_t baudRate, int rxPin, int txPin)
    {
        if (uart_)
        {
            delete uart_;
            uart_ = nullptr;
        }
        uart_ = new SerialTransportChannel(serial, baudRate, rxPin, txPin);
        TransportConfig cfg{};
        cfg.uri = "serial://uart2";
        cfg.clientId = "esp32-uart-bridge";
        cfg.keepAliveSec = 30;
        uart_->begin(cfg);
        uart_->setForwardRawMessages(true);
        transportHub_.addChannel("uart", uart_);
    }

    void SmartSysApp::applySettingsToRuntime(const iotsmartsys::core::settings::Settings &)
    {
        return;
    }

    void SmartSysApp::onMqttConnected(const core::TransportConnectedView &info)
    {
        logger_.debug("MQTT connected.");
        logger_.debug("Client ID: %s", info.clientId);
        logger_.debug("Broker: %s", info.broker);
        logger_.debug("Keep Alive: %d", info.keepAliveSec);

        app::AnnouncePayloadBuilder builder(
            capabilityManager_->getAllCapabilities(), logger_);

        builder.withDeviceId(info.clientId)
            .withBroker(info.broker)
            .withVersion(IOTSMARTSYSCORE_VERSION)
            .withBuild(getBuildIdentifier())
            .withProperty(Property("wifi_ssid", wifi_.getSsid()))
            .withProperty(Property("wifi_signal", wifi_.getSignalStrength()))
            .withIpAddress(wifi_.getIpAddress())
            .withMacAddress(wifi_.getMacAddress());

        std::string payload = builder.build();
        logger_.info("Publishing MQTT message: %s", payload.c_str());

        core::settings::Settings currentSettings;
        if (!settingsManager_.copyCurrent(currentSettings))
        {
            logger_.error("Failed to copy current settings for MQTT publish.");
            return;
        }

        std::string topic = currentSettings.mqtt.announce_topic;
        if (mqtt_.publish(
                topic.c_str(),
                payload.c_str(),
                payload.length(),
                false))
        {
            logger_.info("MQTT message published successfully in topic: %s", topic.c_str());
        }
        else
        {
            logger_.error("Failed to publish MQTT message.");
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
        serviceManager_.setLogLevel(core::LogLevel::Debug);
        core::Log::setLogger(&logger_);

        settingsManager_.setUpdatedCallback(SmartSysApp::onSettingsUpdatedThunk, this);

        const auto cacheErr = settingsManager_.init();

        if (cacheErr == iotsmartsys::core::common::StateResult::Ok)
        {
            if (settingsManager_.copyCurrent(settings_))
            {
                logger_.info("[SettingsManager] Cached settings loaded successfully.");
                logger_.info("---------------------------------------------------------");
                logger_.info("[SettingsManager]", " Firmware Update Mode: %d", (int)settings_.firmware.update);

                logger_.info("[SettingsManager]", " OTA URL: %s", settings_.firmware.url.c_str());
                logger_.info("[SettingsManager]", " OTA Version: %s", getBuildIdentifier());
                logger_.info("[SettingsManager]", "Library Version: %s", IOTSMARTSYSCORE_VERSION);

                logger_.info("[SettingsManager] Log Level: %s", settings_.logLevelStr());
                logger_.info("[SettingsManager] WiFi SSID: %s", settings_.wifi.ssid.c_str());
                logger_.info("[SettingsManager] WiFi Password: %s", settings_.wifi.password.c_str());
                logger_.info("[SettingsManager] API Key: %s", settings_.api.key.c_str());
                logger_.info("[SettingsManager] API URL: %s", settings_.api.url.c_str());
                logger_.info("[SettingsManager] Api Basic Auth: %s", settings_.api.basic_auth.c_str());


                logger_.info("[SettingsManager] In Config Mode: %s", settings_.in_config_mode ? "Yes" : "No");
             
                auto running = esp_ota_get_running_partition();
                auto bootp = esp_ota_get_boot_partition();
                Serial.printf("BOOT:    %s @ 0x%06lX\n", bootp->label, (unsigned long)bootp->address);
                Serial.printf("RUNNING: %s @ 0x%06lX\n", running->label, (unsigned long)running->address);

                logger_.info("----------------------------------------------------------");

                serviceManager_.setLogLevel(LogLevel::Info);

                delay(3000);

                delay(3000);

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

        iotsmartsys::core::ConnectivityGate::init(latch_);

        static iotsmartsys::core::CapabilityManager capManager = builder_.build();
        capabilityManager_ = &capManager;
        capabilityManager_->setup();
        commandProcessorFactory_ = new core::CommandProcessorFactory(logger_, *capabilityManager_);
        commandDispatcher_ = new CapabilityCommandTransportDispatcher(*commandProcessorFactory_, commandParser_, logger_);

        char topic[128];
        snprintf(topic, sizeof(topic), "device/%s/command",
                 settings_.clientId ? settings_.clientId : "");

        mqtt_.subscribe(topic);
        mqtt_.setOnConnected(&SmartSysApp::onMqttConnectedThunk, this);
        mqtt_.setForwardRawMessages(true);

        // Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

        // TransportConfig cfg{};
        // cfg.uri = "serial://uart2";
        // cfg.clientId = "esp32-uart-bridge";
        // cfg.keepAliveSec = 30;
        // uart_.begin(cfg);
        // // uart_.setForwardRawMessages(true);
        // transportHub_.addChannel("uart", &uart_);
        transportHub_.addChannel("mqtt", &mqtt_);
        transportHub_.addDispatcher(*commandDispatcher_);
        // Opcional

        transportHub_.start();
    }

    void SmartSysApp::handle()
    {
        if (inConfigMode_ && provManager)
        {
            provManager->handle();
            return;
        }

        if (factoryResetButton_)
        {
            factoryResetButton_->handle();
            if (factoryResetButton_->getState() == SWITCH_STATE_ON && factoryResetButton_->lastStateReadMillis() > 5000)
            {
                logger_.warn("Factory reset button pressed. Clearing settings and restarting...");
                settingsManager_.clear();
                delay(2000);
                ESP.restart();
            }
        }

        if (capabilityManager_)
        {
            capabilityManager_->handle();
        }

        wifi_.handle();
        otaManager_.handle();
        settingsManager_.handle();
        transportHub_.handle();
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
                                                 sp_.settingsManager().save(newSettings);
                                                 ESP.restart(); });

        provManager->begin();
        inConfigMode_ = true;
    }

    void SmartSysApp::onMqttConnectedThunk(void *ctx, const core::TransportConnectedView &info)
    {
        if (!ctx)
        {
            return;
        }
        static_cast<SmartSysApp *>(ctx)->onMqttConnected(info);
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

    /// @brief Adds a new GLP meter Percent capability to the application.
    iotsmartsys::core::GlpMeterPercentCapability *SmartSysApp::addGlpMeterPercentCapability(iotsmartsys::app::GlpMeterConfig cfg)
    {
        return builder_.addGlpMeterPercent(cfg);
    }

    /// @brief Adds a new GLP meter Kg capability to the application.
    iotsmartsys::core::GlpMeterKgCapability *SmartSysApp::addGlpMeterKgCapability(iotsmartsys::app::GlpMeterConfig cfg)
    {
        return builder_.addGlpMeterKg(cfg);
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

    /// @brief Adds a new luminosity sensor capability to the application.
    iotsmartsys::core::LuminosityCapability *SmartSysApp::addLuminosityCapability(iotsmartsys::app::LuminositySensorConfig cfg)
    {
        return builder_.addLuminosityCapability(cfg);
    }

} // namespace iotsmartsys
