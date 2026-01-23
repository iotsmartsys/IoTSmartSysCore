#include "SmartSysApp.h"
#include "App/Builders/Builders/AnnouncePayloadBuilder.h"
#include "Version/VersionInfo.h"

#include "Platform/Esp8266/Mqtt/Esp8266PubSubMqttClient.h"

#include <cstdio>
#include <memory>
using namespace iotsmartsys::platform::espressif::ota;

namespace iotsmartsys
{
    SmartSysApp::SmartSysApp()
        : serviceManager_(core::ServiceManager::init()),
          logger_(serviceManager_.logger()),
          settingsManager_(serviceManager_.settingsManager()),
          settingsGate_(serviceManager_.settingsGate()),
          commandParser_(logger_),
#ifdef ESP8266
          mqttClient_(std::make_unique<platform::esp8266::Esp8266PubSubMqttClient>(logger_)),
#else
          mqttClient_(std::make_unique<platform::espressif::EspIdfMqttClient>(logger_)),
#endif
          mqtt_(*mqttClient_, logger_, settingsGate_, settingsManager_),
          mqttSink_(mqtt_, settingsManager_),
          deviceStateManager_(logger_, hwFactory_, wifi_, provisioningController_),
          builder_(hwFactory_,
                   mqttSink_,
                   capSlots_,
                   capDtors_,
                   8,
                   adapterSlots_,
                   adapterDtors_,
                   8,
                   arena_,
                   sizeof(arena_),
                   deviceIdentityProvider_),
          wifi_(logger_),
          connectivityBootstrap_(logger_, serviceManager_, settingsManager_, wifi_),
          manifestParser_(),
#ifndef OTA_DISABLED
          ota_(logger_, deviceIdentityProvider_),
#endif
          otaManager_(settingsManager_, logger_, manifestParser_,
#ifndef OTA_DISABLED
                      ota_,
#endif
                      settingsGate_),
          systemCommandProcessor_(logger_),
          factoryResetButtonController_(logger_, settingsManager_, systemCommandProcessor_, hwFactory_),
          capabilityController_(logger_, commandParser_, systemCommandProcessor_),
          transportController_(logger_, settingsManager_, mqtt_),
          provisioningController_(logger_, wifi_, deviceIdentityProvider_)
    {
    }

    void SmartSysApp::configureFactoryResetButton(iotsmartsys::app::PushButtonConfig cfg)
    {
        factoryResetButtonController_.configure(cfg);
    }

    void SmartSysApp::configureLED(iotsmartsys::app::LightConfig cfg)
    {
        deviceStateManager_.configure(cfg);
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
        transportController_.addChannel("uart", uart_);
    }

    void SmartSysApp::applySettingsToRuntime(const iotsmartsys::core::settings::Settings &)
    {
        systemCommandProcessor_.restartSafely();
    }

    void SmartSysApp::onMqttConnected(const core::TransportConnectedView &info)
    {
        logger_.info("MQTT connected to broker: %s", info.broker);
        logger_.info("MQTT client ID: %s", info.clientId);
        logger_.info("MQTT keep alive: %d", info.keepAliveSec);
        
        
        auto *capabilityManager = capabilityController_.manager();
        if (!capabilityManager)
        {
            logger_.error("CapabilityManager not initialized; skipping announce.");
            return;
        }
        app::AnnouncePayloadBuilder builder(
            capabilityManager->getAllCapabilities(), logger_);

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
            settings_ = newSettings;
            logger_.warn("[SettingsManager] Settings have changes. Applying...");
            applySettingsToRuntime(newSettings);
        }
        else
        {
            logger_.warn("[SettingsManager] Settings have NO changes. Skipping apply.");
            return;
        }
    }

    void SmartSysApp::setup()
    {
        serviceManager_.setLogLevel(core::LogLevel::Info);
        core::Log::setLogger(&logger_);
        delay(3000);
        logger_.info("---------------------------------------------------------");
        logger_.info("IoT SmartSys Core Version: ", IOTSMARTSYSCORE_VERSION);
        logger_.info("Device ID: ", deviceIdentityProvider_.getDeviceID().c_str());
        logger_.info("---------------------------------------------------------");

        settingsManager_.setUpdatedCallback(SmartSysApp::onSettingsUpdatedThunk, this);

        const auto path = connectivityBootstrap_.run(settings_);
        if (path == app::ConnectivityBootstrap::BootPath::Provisioning)
        {
            provisioningController_.begin();
            return;
        }

        iotsmartsys::core::ConnectivityGate::init(latch_);



        capabilityController_.setup(builder_);
        transportController_.addDispatcher(*capabilityController_.dispatcher());
        transportController_.configureMqtt(
            deviceIdentityProvider_.getDeviceID().c_str(),
            &SmartSysApp::onMqttConnectedThunk,
            this);
        transportController_.start();
    }

    void SmartSysApp::handle()
    {
        // deviceStateManager_.handle();
        if (provisioningController_.isActive())
        {
            provisioningController_.handle();
            return;
        }

        factoryResetButtonController_.handle();

        capabilityController_.handle();

        wifi_.handle();
        otaManager_.handle();
        settingsManager_.handle();
        transportController_.handle();
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

    /// @brief Adds a new switch plug capability to the application.
    iotsmartsys::core::SwitchPlugCapability *SmartSysApp::addSwitchPlugCapability(iotsmartsys::app::SwitchConfig cfg)
    {
        return builder_.addSwitchPlug(cfg);
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

    /// @brief Adds a new water flow hall sensor capability to the application.
    iotsmartsys::core::WaterFlowHallSensorCapability *SmartSysApp::addWaterFlowHallSensorCapability(iotsmartsys::app::WaterFlowHallSensorConfig cfg)
    {
        return builder_.addWaterFlowHallSensor(cfg);
    }

    /// @brief Adds a new luminosity sensor capability to the application.
    iotsmartsys::core::LuminosityCapability *SmartSysApp::addLuminosityCapability(iotsmartsys::app::LuminositySensorConfig cfg)
    {
        return builder_.addLuminosityCapability(cfg);
    }

} // namespace iotsmartsys
