#include "SmartSysApp.h"
#include "App/Builders/Builders/AnnouncePayloadBuilder.h"
#include "Version/VersionInfo.h"
#if IOTSMARTSYS_OTA_ENABLED
#include "Infra/OTA/OTA.h"
#include "Infra/OTA/OTAManager.h"
#include "Platform/Espressif/Parsers/EspIdFirmwareManifestParser.h"
#endif

#include <cstdio>
#include <memory>
namespace iotsmartsys
{
    SmartSysApp::SmartSysApp()
        : serviceManager_(core::ServiceManager::init()),
          logger_(serviceManager_.logger()),
          settingsManager_(serviceManager_.settingsManager()),
          settingsGate_(serviceManager_.settingsGate()),
          commandParser_(logger_),
          mqttClient_(std::make_unique<platform::espressif::EspIdfMqttClient>(logger_)),
          mqtt_(*mqttClient_, logger_, settingsGate_, settingsManager_),
          mqttSink_(mqtt_, settingsManager_),
          hwFactory_(),
          deviceIdentityProvider_(),
          wifi_(logger_),
          provisioningController_(logger_, wifi_, deviceIdentityProvider_),
          deviceStateManager_(logger_, hwFactory_, wifi_, provisioningController_),
          connectivityBootstrap_(logger_, serviceManager_, settingsManager_, wifi_),
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
          systemCommandProcessor_(logger_),
          factoryResetButtonController_(logger_, settingsManager_, systemCommandProcessor_, hwFactory_),
          capabilityController_(logger_, commandParser_, systemCommandProcessor_),
          transportController_(logger_, settingsManager_, mqtt_),
          uart_(nullptr)
    {
#if IOTSMARTSYS_OTA_ENABLED
        manifestParser_ = std::make_unique<platform::espressif::ota::EspIdFirmwareManifestParser>();
#ifndef OTA_DISABLED
        ota_ = std::make_unique<ota::OTA>(logger_, deviceIdentityProvider_);
#endif
        otaManager_ = std::make_unique<ota::OTAManager>(
            settingsManager_,
            logger_,
            *manifestParser_,
#ifndef OTA_DISABLED
            *ota_,
#endif
            settingsGate_);
#endif
    }

    SmartSysApp::~SmartSysApp() = default;

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
        uart_ = new core::SerialTransportChannel(serial, baudRate, rxPin, txPin);
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
        auto *capabilityManager = capabilityController_.manager();
        if (!capabilityManager)
        {
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

        core::settings::Settings currentSettings;
        if (!settingsManager_.copyCurrent(currentSettings))
        {
            return;
        }

        std::string topic = currentSettings.mqtt.announce_topic;
        mqtt_.publish(topic.c_str(), payload.c_str(), payload.length(), false);
    }

    void SmartSysApp::onSettingsUpdated(const iotsmartsys::core::settings::Settings &newSettings)
    {
        if (newSettings.hasChanges())
        {
            settings_ = newSettings;
            applySettingsToRuntime(newSettings);
        }
    }

    void SmartSysApp::setup()
    {
        serviceManager_.setLogLevel(core::LogLevel::Info);
        core::Log::setLogger(&logger_);
        delay(3000);
        logger_.info("---------------------------------------------------------");
        logger_.info("IoT SmartSys Core Version: ", IOTSMARTSYSCORE_VERSION);
        logger_.info("Device ID", deviceIdentityProvider_.getDeviceID().c_str());
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
        deviceStateManager_.handle();
        if (provisioningController_.isActive())
        {
            provisioningController_.handle();
        }

        factoryResetButtonController_.handle();

        capabilityController_.handle();

        wifi_.handle();
#if IOTSMARTSYS_OTA_ENABLED
#if IOTSMARTSYS_OTA_ENABLED
        if (otaManager_)
        {
            otaManager_->handle();
        }
#endif
#endif
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
