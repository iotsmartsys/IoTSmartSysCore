#pragma once

#include <Arduino.h>

#include "Platform/Arduino/Sensors/ACS712C30ACurrentSensor.h"
#include "Platform/Arduino/Sensors/ResistiveDividerVoltageSensor.h"
#include "SmartSysApp.h"

#ifndef ITS_MCB01_J4_EXT_ADC
#error "power_energy requires the MCB R1 pinout symbol ITS_MCB01_J4_EXT_ADC"
#endif

#ifndef ITS_MCB01_J4_EXT_IO33
#error "power_energy requires the MCB R1 pinout symbol ITS_MCB01_J4_EXT_IO33"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "power_energy requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

#ifndef EXAMPLE_CURRENT_SENSOR_ID
#error "power_energy requires EXAMPLE_CURRENT_SENSOR_ID to be defined"
#endif

#ifndef EXAMPLE_VOLTAGE_SENSOR_ID
#error "power_energy requires EXAMPLE_VOLTAGE_SENSOR_ID to be defined"
#endif

#ifndef EXAMPLE_POWER_ENERGY_ID
#error "power_energy requires EXAMPLE_POWER_ENERGY_ID to be defined"
#endif

#ifndef EXAMPLE_POWER_ENERGY_LOG_INTERVAL_MS
#error "power_energy requires EXAMPLE_POWER_ENERGY_LOG_INTERVAL_MS to be defined"
#endif

namespace executable_example
{
    using iotsmartsys::core::CurrentSensorConfig;
    using iotsmartsys::core::PowerEnergyCapability;
    using iotsmartsys::core::PowerEnergyConfig;
    using iotsmartsys::core::PowerEnergyMeasurement;
    using iotsmartsys::core::VoltageSensorConfig;
    using iotsmartsys::platform::arduino::ACS712C30ACurrentSensor;
    using iotsmartsys::platform::arduino::ResistiveDividerVoltageSensor;

    static CurrentSensorConfig buildCurrentConfig()
    {
        return CurrentSensorConfig::ACS712_30A_3V3(
            EXAMPLE_CURRENT_SENSOR_ID,
            ITS_MCB01_J4_EXT_ADC);
    }

    static VoltageSensorConfig buildVoltageConfig()
    {
        VoltageSensorConfig config;
        config.id = EXAMPLE_VOLTAGE_SENSOR_ID;
        config.adcPin = ITS_MCB01_J4_EXT_IO33;
        config.adcMinimumMv = 144.0f;
        config.r1Ohms = 330000.0f;
        config.r2Ohms = 10000.0f;
        return config;
    }

    // Construction order makes both externally owned sensors outlive the app
    // and, consequently, the capability that keeps references to them.
    static CurrentSensorConfig currentConfig = buildCurrentConfig();
    static VoltageSensorConfig voltageConfig = buildVoltageConfig();
    static ACS712C30ACurrentSensor currentSensor(currentConfig);
    static ResistiveDividerVoltageSensor voltageSensor(voltageConfig);
    static iotsmartsys::SmartSysApp app;
    static PowerEnergyCapability *powerEnergy = nullptr;
    static unsigned long lastReportMs = 0;

    static void reportMeasurement()
    {
        const PowerEnergyMeasurement &measurement = powerEnergy->powerEnergyMeasurement();
        const char *status = iotsmartsys::core::toString(measurement.measurementStatus);

        if (measurement.powerW.has_value())
        {
            Serial.printf("[example] power=%.2f W energy=%.3f Wh measurementStatus=%s\n",
                          measurement.powerW.value(), measurement.energyWh, status);
        }
        else
        {
            Serial.printf("[example] power=<none> energy=%.3f Wh measurementStatus=%s\n",
                          measurement.energyWh, status);
        }
    }

    static void handleOperatorInput()
    {
        while (Serial.available() > 0)
        {
            const int command = Serial.read();
            if (command == 'r' || command == 'R')
            {
                powerEnergy->resetEnergy();
                Serial.println("[example] local energy reset requested");
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);

    executable_example::PowerEnergyConfig config;
    config.id = EXAMPLE_POWER_ENERGY_ID;
    config.readingIntervalMs = 1000;

    Serial.printf(
        "[example] id=power_energy board=%s current_pin_symbol=ITS_MCB01_J4_EXT_ADC "
        "current_gpio=%d current_profile=ACS712_30A_3V3 voltage_pin_symbol=ITS_MCB01_J4_EXT_IO33 "
        "voltage_gpio=%d current_sensor=%s voltage_sensor=%s power_energy=%s interval_ms=%lu\n",
        EXAMPLE_BOARD_ID,
        ITS_MCB01_J4_EXT_ADC,
        ITS_MCB01_J4_EXT_IO33,
        executable_example::currentConfig.id.c_str(),
        executable_example::voltageConfig.id.c_str(),
        config.id.c_str(),
        static_cast<unsigned long>(config.readingIntervalMs));
    Serial.printf("[example] current_supply_nominal=%.0f mV supply_monitor_pin=%d\n",
                  static_cast<double>(executable_example::currentConfig.supplyNominalMv),
                  executable_example::currentConfig.supplyMonitorAdcPin);
    Serial.printf("[example] voltage_divider R1=%.0f ohm R2=%.0f ohm adc_minimum=%.0f mV\n",
                  static_cast<double>(executable_example::voltageConfig.r1Ohms),
                  static_cast<double>(executable_example::voltageConfig.r2Ohms),
                  static_cast<double>(executable_example::voltageConfig.adcMinimumMv));
    Serial.println("[example] current supply monitoring disabled; composed numeric state is ESTIMATED");
    Serial.println("[example] send 'r' to reset accumulated energy locally");

    executable_example::currentSensor.setup();
    executable_example::voltageSensor.setup();

    executable_example::powerEnergy = executable_example::app.addPowerEnergyCapability(
        config,
        executable_example::voltageSensor,
        executable_example::currentSensor);

    if (executable_example::powerEnergy == nullptr)
        Serial.println("[example] addPowerEnergyCapability failed; see the runtime log for the cause");

    executable_example::app.setup();
}

void loop()
{
    executable_example::currentSensor.handle();
    executable_example::voltageSensor.handle();
    executable_example::app.handle();

    if (executable_example::powerEnergy == nullptr)
        return;

    executable_example::handleOperatorInput();

    const unsigned long nowMs = millis();
    if (nowMs - executable_example::lastReportMs < EXAMPLE_POWER_ENERGY_LOG_INTERVAL_MS)
        return;

    executable_example::lastReportMs = nowMs;
    executable_example::reportMeasurement();
}
