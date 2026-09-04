#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <memory>

#include "Platform/Arduino/Adapters/OutputHardwareAdapter.h"
#include "Platform/Arduino/Sensors/ACS712C30ACurrentSensor.h"
#include "Platform/Arduino/Sensors/ResistiveDividerVoltageSensor.h"
#include "SmartSysApp.h"

#ifndef EXAMPLE_BOARD_ID
#error "mcb01_solar_controller requires EXAMPLE_BOARD_ID"
#endif

#if IOTSMARTSYS_MAX_CAPABILITIES != 12
#error "mcb01_solar_controller requires exactly 12 capability slots"
#endif

namespace executable_example
{
    using namespace iotsmartsys::app;
    using namespace iotsmartsys::core;
    using namespace iotsmartsys::infra::factories;
    using namespace iotsmartsys::platform::arduino;

    constexpr std::uint8_t kBatteryChargingChannel = 1;
    constexpr std::uint8_t kBatteryDischargingChannel = 0;

    template <typename T>
    inline constexpr std::size_t kAlignedAllocationUpperBound = sizeof(T) + alignof(T) - 1U;

    inline constexpr std::size_t kMcb01ArenaUpperBound =
        2U * kAlignedAllocationUpperBound<OutputHardwareAdapter> +
        3U * kAlignedAllocationUpperBound<CurrentSensorCapability> +
        2U * kAlignedAllocationUpperBound<VoltageSensorCapability> +
        kAlignedAllocationUpperBound<PowerEnergyCapability> +
        kAlignedAllocationUpperBound<SwitchCapability> +
        kAlignedAllocationUpperBound<FanCapability> +
        kAlignedAllocationUpperBound<TemperatureSensorCapability>;

    static_assert(iotsmartsys::config::kCapabilityArenaBytes >= kMcb01ArenaUpperBound,
                  "MCB01 capability arena is smaller than its nine-capability composition");

    static INA3221Device inaDevice(Wire, INA3221DeviceConfig::createDeviceConfig());
    static INA3221VoltageSensor batteryChargingVoltage(
        inaDevice, INA3221VoltageSensor::createVoltageConfig(kBatteryChargingChannel));
    static INA3221CurrentSensor batteryChargingCurrent(
        inaDevice, INA3221CurrentSensor::createCurrentConfig(kBatteryChargingChannel));
    static INA3221CurrentSensor batteryDischargingCurrent(
        inaDevice, INA3221CurrentSensor::createCurrentConfig(kBatteryDischargingChannel));

    static CurrentSensorConfig pvCurrentConfig =
        CurrentSensorConfig::ACS712_30A_5V("pv-current-1", ITS_MCB01_J4_EXT_ADC);
    static VoltageSensorConfig pvVoltageConfig =
        VoltageSensorConfig::createResistiveDivider330KVoltageConfig(
            "pv-voltage-1", ITS_MCB01_J4_EXT_IO33);
    static ACS712C30ACurrentSensor pvCurrent(pvCurrentConfig);
    static ResistiveDividerVoltageSensor pvVoltage(pvVoltageConfig);

    static iotsmartsys::SmartSysApp app;
    static SensorFactory sensorFactory(ServiceManager::init().logger());
    static std::unique_ptr<ITemperatureSensor> temperatureSensor;
    static bool appReady = false;

    static bool requireRegistration(const char *id, const void *capability)
    {
        if (capability != nullptr)
        {
            return true;
        }
        Serial.printf("[mcb01] capability registration failed: %s\n", id);
        return false;
    }
}

void setup()
{
    using namespace executable_example;

    Serial.begin(115200);
    delay(500);
    Wire.begin(ESP32_SDA, ESP32_SCL);

    temperatureSensor = sensorFactory.createTemperatureSensor(
        ITS_MCB01_TEMPERATURE_SENSOR_PIN, TemperatureSensorModel::DS18B20);
    TemperatureSensorConfig temperatureConfig;
    temperatureConfig.readIntervalMs = 30000;
    temperatureConfig.sensor = temperatureSensor.get();

    PowerEnergyConfig pvPowerConfig;
    pvPowerConfig.id = "pv-power-1";
    pvPowerConfig.readingIntervalMs = 1000;

    bool registered = true;
    registered &= requireRegistration(
        "pv-current-1", app.addCurrentSensor("pv-current-1", pvCurrent, 15000));
    registered &= requireRegistration(
        "pv-voltage-1", app.addVoltageSensor("pv-voltage-1", pvVoltage, 1000));
    registered &= requireRegistration(
        "pv-power-1", app.addPowerEnergyCapability(pvPowerConfig, pvVoltage, pvCurrent));
    registered &= requireRegistration(
        "controller-power",
        app.addSwitchCapability(SwitchConfig{ITS_MCB01_BUZZER_PIN, false, "controller-power"}));
    registered &= requireRegistration(
        "controller-fan_1",
        app.addFanCapability(FanConfig{ITS_MCB01_RELAY_PIN, true, "controller-fan_1"}));
    registered &= requireRegistration(
        "temperature", app.addTemperatureSensorCapability(temperatureConfig));
    registered &= requireRegistration(
        "battery-charging-voltage",
        app.addVoltageSensor("battery-charging-voltage", batteryChargingVoltage, 1000));
    registered &= requireRegistration(
        "battery-charging-current",
        app.addCurrentSensor("battery-charging-current", batteryChargingCurrent, 1000));
    registered &= requireRegistration(
        "battery-discharging-current",
        app.addCurrentSensor("battery-discharging-current", batteryDischargingCurrent, 1000));

    if (!registered)
    {
        Serial.println("[mcb01] setup aborted: the nine-capability set is incomplete");
        return;
    }

    app.configureLED(LightConfig{ITS_MCB01_LED_PIN});
    app.configureFactoryResetButton(PushButtonConfig{ESP32_BOOT_PIN});
    app.setup();
    appReady = true;
    Serial.printf("[mcb01] setup completed: capabilities=9 capacity=%u board=%s\n",
                  static_cast<unsigned>(IOTSMARTSYS_MAX_CAPABILITIES), EXAMPLE_BOARD_ID);
}

void loop()
{
    if (executable_example::appReady)
    {
        executable_example::app.handle();
    }
}
