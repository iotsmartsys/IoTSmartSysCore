#pragma once

#include "pins.h"
#include <Arduino.h>
#include <memory>
#include <string>
#include "Config/BuildConfig.h"
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
#include "App/Managers/CapabilityController.h"
#include "App/Managers/ConnectivityBootstrap.h"
#include "App/Managers/FactoryResetButtonController.h"
#include "App/Managers/DeviceStateManager.h"
#include "App/Managers/ProvisioningController.h"
#include "App/Managers/TransportController.h"

#include "Infra/Factories/SensorFactory.h"

#include "Platform/Arduino/Transports/ArduinoSerialTransportChannel.h"
#include "Platform/Espressif/Providers/DeviceIdentityProvider.h"
#include "Contracts/Mqtt/IMqttClient.h"

namespace iotsmartsys::ota
{
        class OTA;
        class OTAManager;
}

namespace iotsmartsys::platform::espressif::ota
{
        class EspIdFirmwareManifestParser;
}

namespace iotsmartsys
{
        class SmartSysApp
        {
        public:
                SmartSysApp();
                ~SmartSysApp();

                /// @brief Initializes the application, including hardware and services.
                void setup();

                /// @brief Main application loop to be called repeatedly.
                void handle();

                /// @brief Configure SerialTransportChannel UART pins and baud rate.
                void configureSerialTransport(HardwareSerial &serial, uint32_t baudRate, int rxPin, int txPin);

                /// @brief Configura botão de reset de fábrica (provisionamento).
                void configureFactoryResetButton(iotsmartsys::app::PushButtonConfig cfg);

                void configureLED(iotsmartsys::app::LightConfig cfg);

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
                iotsmartsys::core::SwitchPlugCapability *addSwitchPlugCapability(iotsmartsys::app::SwitchConfig cfg);
                iotsmartsys::core::ValveCapability *addValveCapability(iotsmartsys::app::ValveConfig cfg);
                iotsmartsys::core::OperationalColorSensorCapability *addOperationalColorSensorCapability(iotsmartsys::app::OperationalColorSensorConfig cfg);
                iotsmartsys::core::TemperatureSensorCapability *addTemperatureSensorCapability(iotsmartsys::app::TemperatureSensorConfig cfg);
                iotsmartsys::core::WaterLevelLitersCapability *addWaterLevelLitersCapability(iotsmartsys::app::WaterLevelSensorConfig cfg);
                iotsmartsys::core::WaterLevelPercentCapability *addWaterLevelPercentCapability(iotsmartsys::app::WaterLevelSensorConfig cfg);
                iotsmartsys::core::WaterFlowHallSensorCapability *addWaterFlowHallSensorCapability(iotsmartsys::app::WaterFlowHallSensorConfig cfg);
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

                std::unique_ptr<iotsmartsys::core::IMqttClient> mqttClient_;

                iotsmartsys::core::settings::Settings settings_;
                app::MqttService<12, 16, 256> mqtt_;
                iotsmartsys::core::MqttSink mqttSink_;
                iotsmartsys::platform::arduino::ArduinoHardwareAdapterFactory hwFactory_;
                platform::espressif::providers::DeviceIdentityProvider deviceIdentityProvider_;

                core::WiFiManager wifi_;
                app::ProvisioningController provisioningController_;
                app::DeviceStateManager deviceStateManager_;
                app::ConnectivityBootstrap connectivityBootstrap_;

                core::ICapability *capSlots_[8] = {};
                void (*capDtors_[8])(void *) = {};
                void *adapterSlots_[8] = {};
                void (*adapterDtors_[8])(void *) = {};
                uint8_t arena_[2048] = {};

                app::CapabilitiesBuilder builder_;

                iotsmartsys::platform::espressif::arduino::ArduinoEventLatch latch_;

#if IOTSMARTSYS_OTA_ENABLED
                std::unique_ptr<iotsmartsys::platform::espressif::ota::EspIdFirmwareManifestParser> manifestParser_;
#ifndef OTA_DISABLED
                std::unique_ptr<iotsmartsys::ota::OTA> ota_;
#endif
                std::unique_ptr<iotsmartsys::ota::OTAManager> otaManager_;
#endif

                iotsmartsys::core::SystemCommandProcessor systemCommandProcessor_;
                app::FactoryResetButtonController factoryResetButtonController_;
                app::CapabilityController capabilityController_;
                app::TransportController transportController_;
                core::SerialTransportChannel *uart_;
        };
} // namespace iotsmartsys
