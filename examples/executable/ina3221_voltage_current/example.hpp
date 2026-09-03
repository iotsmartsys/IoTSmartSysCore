#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "SmartSysApp.h"

#ifndef ESP32_SDA
#error "ina3221_voltage_current requires the official ESP32_SDA symbol"
#endif

#ifndef ESP32_SCL
#error "ina3221_voltage_current requires the official ESP32_SCL symbol"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "ina3221_voltage_current requires EXAMPLE_BOARD_ID"
#endif

#ifndef EXAMPLE_INA3221_VOLTAGE_ID
#error "ina3221_voltage_current requires EXAMPLE_INA3221_VOLTAGE_ID"
#endif

#ifndef EXAMPLE_INA3221_CURRENT_ID
#error "ina3221_voltage_current requires EXAMPLE_INA3221_CURRENT_ID"
#endif

#ifndef EXAMPLE_INA3221_LOG_INTERVAL_MS
#error "ina3221_voltage_current requires EXAMPLE_INA3221_LOG_INTERVAL_MS"
#endif

#if EXAMPLE_INA3221_LOG_INTERVAL_MS < 1000
#error "ina3221_voltage_current reporting interval must be at least 1000 ms"
#endif

namespace executable_example
{
    using iotsmartsys::core::CurrentMeasurement;
    using iotsmartsys::core::CurrentSensorCapability;
    using iotsmartsys::core::VoltageMeasurement;
    using iotsmartsys::core::VoltageSensorCapability;
    using iotsmartsys::platform::arduino::INA3221Averaging;
    using iotsmartsys::platform::arduino::INA3221ConversionTime;
    using iotsmartsys::platform::arduino::INA3221CurrentSensor;
    using iotsmartsys::platform::arduino::INA3221CurrentSensorConfig;
    using iotsmartsys::platform::arduino::INA3221Device;
    using iotsmartsys::platform::arduino::INA3221DeviceConfig;
    using iotsmartsys::platform::arduino::INA3221VoltageSensor;
    using iotsmartsys::platform::arduino::INA3221VoltageSensorConfig;

    static INA3221DeviceConfig buildDeviceConfig()
    {
        INA3221DeviceConfig config;
        config.i2cAddress = 0x40;
        config.averaging = INA3221Averaging::SAMPLES_16;
        config.busConversionTime = INA3221ConversionTime::MS_1;
        config.shuntConversionTime = INA3221ConversionTime::MS_1;
        return config;
    }

    static INA3221VoltageSensorConfig buildVoltageConfig()
    {
        INA3221VoltageSensorConfig config;
        config.channel = 0;
        config.minimumVoltageV = 0.0f;
        config.maximumVoltageV = 26.0f;
        config.readingIntervalMs = 500;
        return config;
    }

    static INA3221CurrentSensorConfig buildCurrentConfig()
    {
        INA3221CurrentSensorConfig config;
        config.channel = 0;
        config.shuntResistanceOhms = 0.100f;
        config.polarity = 1.0f;
        config.deadbandA = 0.005f;
        config.minimumReportableA = 0.010f;
        config.maximumAbsoluteCurrentA = 1.500f;
        config.readingIntervalMs = 500;
        return config;
    }

    // Construction order preserves the external ownership chain: the device
    // and adapters outlive SmartSysApp and both capabilities that reference them.
    static INA3221DeviceConfig deviceConfig = buildDeviceConfig();
    static INA3221Device device(Wire, deviceConfig);
    static INA3221VoltageSensorConfig voltageConfig = buildVoltageConfig();
    static INA3221VoltageSensor voltageAdapter(device, voltageConfig);
    static INA3221CurrentSensorConfig currentConfig = buildCurrentConfig();
    static INA3221CurrentSensor currentAdapter(device, currentConfig);
    static iotsmartsys::SmartSysApp app;
    static VoltageSensorCapability *voltageCapability = nullptr;
    static CurrentSensorCapability *currentCapability = nullptr;
    static unsigned long lastReportMs = 0;

    static void reportVoltage()
    {
        if (voltageCapability == nullptr)
        {
            Serial.println("[example] voltage capability unavailable");
            return;
        }

        const VoltageMeasurement &measurement = voltageCapability->voltageMeasurement();
        if (measurement.voltageV.has_value())
        {
            Serial.printf("[example] voltage=%.2f V measurementStatus=%s\n",
                          static_cast<double>(measurement.voltageV.value()),
                          iotsmartsys::core::toString(measurement.measurementStatus));
        }
        else
        {
            Serial.printf("[example] voltage=<none> measurementStatus=%s\n",
                          iotsmartsys::core::toString(measurement.measurementStatus));
        }
    }

    static void reportCurrent()
    {
        if (currentCapability == nullptr)
        {
            Serial.println("[example] current capability unavailable");
            return;
        }

        const CurrentMeasurement &measurement = currentCapability->currentMeasurement();
        if (measurement.currentA.has_value())
        {
            Serial.printf("[example] current=%.3f A measurementStatus=%s supplyStatus=%s\n",
                          static_cast<double>(measurement.currentA.value()),
                          iotsmartsys::core::toString(measurement.measurementStatus),
                          iotsmartsys::core::toString(measurement.supplyStatus));
        }
        else
        {
            Serial.printf("[example] current=<none> measurementStatus=%s supplyStatus=%s\n",
                          iotsmartsys::core::toString(measurement.measurementStatus),
                          iotsmartsys::core::toString(measurement.supplyStatus));
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin(ESP32_SDA, ESP32_SCL);

    Serial.printf(
        "[example] id=ina3221_voltage_current board=%s bus=Wire "
        "sda_symbol=ESP32_SDA sda_gpio=%d scl_symbol=ESP32_SCL scl_gpio=%d "
        "address=0x%02X channel=0 shunt=R100 shunt_ohms=%.3f averaging=16 "
        "bus_conversion=1ms shunt_conversion=1ms voltage_capability=%s "
        "current_capability=%s adapter_interval_ms=%lu capability_interval_ms=1000\n",
        EXAMPLE_BOARD_ID,
        ESP32_SDA,
        ESP32_SCL,
        static_cast<unsigned>(executable_example::deviceConfig.i2cAddress),
        static_cast<double>(executable_example::currentConfig.shuntResistanceOhms),
        EXAMPLE_INA3221_VOLTAGE_ID,
        EXAMPLE_INA3221_CURRENT_ID,
        static_cast<unsigned long>(executable_example::voltageConfig.readingIntervalMs));

    executable_example::voltageCapability = executable_example::app.addVoltageSensor(
        EXAMPLE_INA3221_VOLTAGE_ID, executable_example::voltageAdapter, 1000);
    if (executable_example::voltageCapability == nullptr)
    {
        Serial.println("[example] voltage registration failed; see runtime log");
    }

    executable_example::currentCapability = executable_example::app.addCurrentSensor(
        EXAMPLE_INA3221_CURRENT_ID, executable_example::currentAdapter, 1000);
    if (executable_example::currentCapability == nullptr)
    {
        Serial.println("[example] current registration failed; see runtime log");
    }

    executable_example::app.setup();
}

void loop()
{
    executable_example::app.handle();

    const unsigned long nowMs = millis();
    if (nowMs - executable_example::lastReportMs < EXAMPLE_INA3221_LOG_INTERVAL_MS)
    {
        return;
    }

    executable_example::lastReportMs = nowMs;
    executable_example::reportVoltage();
    executable_example::reportCurrent();
}
