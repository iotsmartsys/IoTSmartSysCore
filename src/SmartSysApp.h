#pragma once

#include <Arduino.h>
#include <string>
#include "Platform/Espressif/Pinouts/ESP32_S3_Pinouts.h"

// -----------------------------------------------------------------------------
//  Contracts / Core abstractions
// -----------------------------------------------------------------------------
#include "Contracts/Logging/Log.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

#include "Contracts/Settings/SettingsManager.h"

// -----------------------------------------------------------------------------
//  Platform implementations
// -----------------------------------------------------------------------------
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"

// MQTT client implementation (required before declaring mqttClient)
#include "Platform/Espressif/Mqtt/EspIdfMqttClient.h"
#include "Platform/Espressif/Parsers/EspIdfCommandParser.h"
#include "Platform/Espressif/Arduino/Connectivity/ArduinoEventLatch.h"

// platform-specific settings fetcher/parser/providers are implementation details
// and not required by this header. Include them in .cpp where needed.

// -----------------------------------------------------------------------------
//  App / Core services built on top of contracts
// -----------------------------------------------------------------------------
#include "Contracts/Connections/WiFiManager.h"
#include "Core/Providers/ServiceManager.h"
#include "Core/Sinks/MqttSink.h"
#include "Core/Services/MqttService.h"
#include "Core/Settings/SettingsGateImpl.h"
#include "App/Builders/Builders/CapabilitiesBuilder.h"

#include "Core/Provisioning/ProvisioningManager.h"
#if defined(BLE_PROVISIONING_CHANNEL_ENABLE) && (BLE_PROVISIONING_CHANNEL_ENABLE != 0)
#include "Platform/Espressif/Provisioning/BleProvisioningChannel.h"
#endif
#if defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)
#include "Platform/Arduino/Provisioning/WebPortalProvisioningChannel.h"
#endif

#include "Infra/OTA/OTAManager.h"
#include "Platform/Espressif/Parsers/EspIdFirmwareManifestParser.h"
#include "Infra/Factories/SensorFactory.h"
#include "Core/Commands/CommandProcessorFactory.h"

#include "Platform/Arduino/Transports/ArduinoSerialTransportChannel.h"
#include "Core/Transports/TransportHub.h"
#include "Core/Commands/CapabilityCommandTransportDispatcher.h"
#include "Platform/Espressif/Providers/DeviceIdentityProvider.h"

namespace iotsmartsys
{
    class SmartSysApp
    {
    public:
        SmartSysApp();

        /// @brief Initializes the application, including hardware and services.
        void setup();

        /// @brief Main application loop to be called repeatedly.
        void handle();

        /// @brief Configure SerialTransportChannel UART pins and baud rate.
        void configureSerialTransport(HardwareSerial &serial, uint32_t baudRate, int rxPin, int txPin);

        /// @brief Configura botão de reset de fábrica (provisionamento).
        void configureFactoryResetButton(int pin, bool activeLow = true);

        /* Capabilities */
        iotsmartsys::core::AlarmCapability *addAlarmCapability(iotsmartsys::app::AlarmConfig cfg);
        iotsmartsys::core::DoorSensorCapability *addDoorSensorCapability(iotsmartsys::app::DoorSensorConfig cfg);
        iotsmartsys::core::ClapSensorCapability *addClapSensorCapability(iotsmartsys::app::ClapSensorConfig cfg);
        iotsmartsys::core::LightCapability *addLightCapability(iotsmartsys::app::LightConfig cfg);
        iotsmartsys::core::GlpSensorCapability *addGlpSensorCapability(iotsmartsys::app::GlpSensorConfig cfg);
        iotsmartsys::core::GlpMeterPercentCapability *addGlpMeterPercentCapability(iotsmartsys::app::GlpMeterConfig cfg);
        iotsmartsys::core::GlpMeterKgCapability *addGlpMeterKgCapability(iotsmartsys::app::GlpMeterConfig cfg);
        iotsmartsys::core::HumiditySensorCapability *addHumiditySensorCapability(iotsmartsys::app::HumiditySensorConfig cfg);
        iotsmartsys::core::HeightWaterLevelCapability *addHeightWaterLevelCapability(iotsmartsys::app::WaterLevelSensorConfig cfg);
        iotsmartsys::core::LEDCapability *addLedCapability(iotsmartsys::app::LightConfig cfg);
        iotsmartsys::core::PirSensorCapability *addPirSensorCapability(iotsmartsys::app::PirSensorConfig cfg);
        iotsmartsys::core::PushButtonCapability *addPushButtonCapability(iotsmartsys::app::PushButtonConfig cfg);
        iotsmartsys::core::TouchButtonCapability *addTouchButtonCapability(iotsmartsys::app::TouchButtonConfig cfg);
        iotsmartsys::core::SwitchCapability *addSwitchCapability(iotsmartsys::app::SwitchConfig cfg);
        iotsmartsys::core::ValveCapability *addValveCapability(iotsmartsys::app::ValveConfig cfg);
        iotsmartsys::core::OperationalColorSensorCapability *addOperationalColorSensorCapability(iotsmartsys::app::OperationalColorSensorConfig cfg);
        iotsmartsys::core::TemperatureSensorCapability *addTemperatureSensorCapability(iotsmartsys::app::TemperatureSensorConfig cfg);
        iotsmartsys::core::WaterLevelLitersCapability *addWaterLevelLitersCapability(iotsmartsys::app::WaterLevelSensorConfig cfg);
        iotsmartsys::core::WaterLevelPercentCapability *addWaterLevelPercentCapability(iotsmartsys::app::WaterLevelSensorConfig cfg);
        iotsmartsys::core::LuminosityCapability *addLuminosityCapability(iotsmartsys::app::LuminositySensorConfig cfg);

    private:
        static void onMqttMessageThunk(void *ctx, const core::TransportMessageView &msg);
        static void onMqttConnectedThunk(void *ctx, const core::TransportConnectedView &info);
        void onMqttMessage(const core::TransportMessageView &msg);
        void onMqttConnected(const core::TransportConnectedView &info);
        static void onSettingsUpdatedThunk(const core::settings::Settings &newSettings, void *ctx);
        void onSettingsUpdated(const core::settings::Settings &newSettings);
        void applySettingsToRuntime(const core::settings::Settings &settings);

        core::ServiceManager &serviceManager_;
        core::ILogger &logger_;
        core::settings::SettingsManager &settingsManager_;
        core::settings::ISettingsGate &settingsGate_;

        platform::espressif::EspIdfCommandParser commandParser_;
        platform::espressif::EspIdfMqttClient mqttClient_;

        iotsmartsys::core::settings::Settings settings_;
        iotsmartsys::core::MqttSink mqttSink_;
        iotsmartsys::platform::arduino::ArduinoHardwareAdapterFactory hwFactory_;
        core::ICapability *capSlots_[8] = {};
        void (*capDtors_[8])(void *) = {};
        void *adapterSlots_[8] = {};
        void (*adapterDtors_[8])(void *) = {};
        uint8_t arena_[2048] = {};

        app::CapabilitiesBuilder builder_;

        iotsmartsys::platform::espressif::arduino::ArduinoEventLatch latch_;

        core::WiFiManager wifi_;
        app::MqttService<12, 16, 256> mqtt_;
        iotsmartsys::platform::espressif::ota::EspIdFirmwareManifestParser manifestParser_;
#ifdef OTA_ENABLED
        ota::OTA ota_;
#endif
        ota::OTAManager otaManager_;

        iotsmartsys::core::CapabilityManager *capabilityManager_ = nullptr;
        iotsmartsys::core::provisioning::ProvisioningManager *provManager = nullptr;
#if defined(BLE_PROVISIONING_CHANNEL_ENABLE) && (BLE_PROVISIONING_CHANNEL_ENABLE != 0)
        iotsmartsys::core::provisioning::BleProvisioningChannel *bleChannel = nullptr;
#endif
#if defined(WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE) && (WEB_PORTAL_PROVISIONING_CHANNEL_ENABLE != 0)
        iotsmartsys::core::provisioning::WebPortalProvisioningChannel *webPortalChannel = nullptr;
#endif
        iotsmartsys::core::CommandProcessorFactory *commandProcessorFactory_ = nullptr;
        CapabilityCommandTransportDispatcher *commandDispatcher_ = nullptr;
        TransportHub transportHub_;
        SerialTransportChannel *uart_;
        ICommandHardwareAdapter *factoryResetButton_{nullptr};
        platform::espressif::providers::DeviceIdentityProvider deviceIdentityProvider_;


        void setupProvisioningConfiguration();
        bool inConfigMode_{false};
    };
} // namespace iotsmartsys
