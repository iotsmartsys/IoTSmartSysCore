#pragma once

#include <Arduino.h>
#include "SmartSysApp.h"

#ifndef ITS_MCB01_J4_EXT_ADC
#error "voltage_sensor requires the MCB R1 pinout symbol ITS_MCB01_J4_EXT_ADC"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "voltage_sensor requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

#ifndef EXAMPLE_VOLTAGE_SENSOR_ID
#error "voltage_sensor requires EXAMPLE_VOLTAGE_SENSOR_ID to be defined"
#endif

#ifndef EXAMPLE_VOLTAGE_LOG_INTERVAL_MS
#error "voltage_sensor requires EXAMPLE_VOLTAGE_LOG_INTERVAL_MS to be defined"
#endif

namespace executable_example
{
    using iotsmartsys::core::VoltageMeasurement;
    using iotsmartsys::core::VoltageSensorCapability;
    using iotsmartsys::core::VoltageSensorConfig;

    static iotsmartsys::SmartSysApp app;
    static VoltageSensorCapability *voltageSensor = nullptr;
    static unsigned long lastReportMs = 0;

    static VoltageSensorConfig buildConfig()
    {
        VoltageSensorConfig config;
        config.id = EXAMPLE_VOLTAGE_SENSOR_ID;
        config.adcPin = ITS_MCB01_J4_EXT_ADC;
        config.adcMinimumMv = 144.0f;
        config.r1Ohms = 330000.0f;
        config.r2Ohms = 10000.0f;
        config.capabilityEvaluationIntervalMs = 1000;
        return config;
    }

    static void reportMeasurement()
    {
        const VoltageMeasurement &measurement = voltageSensor->voltageMeasurement();
        const char *status = iotsmartsys::core::toString(measurement.measurementStatus);
        if (measurement.voltageV.has_value())
        {
            Serial.printf("[example] voltage=%.2f V measurementStatus=%s\n",
                          measurement.voltageV.value(), status);
        }
        else
        {
            Serial.printf("[example] voltage=<none> measurementStatus=%s\n", status);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    const iotsmartsys::core::VoltageSensorConfig config = executable_example::buildConfig();
    Serial.printf("[example] id=voltage_sensor board=%s signal_pin=%d capability=%s R1=%.0f R2=%.0f minimum=%.0f mV\n",
                  EXAMPLE_BOARD_ID,
                  ITS_MCB01_J4_EXT_ADC,
                  config.id.c_str(),
                  static_cast<double>(config.r1Ohms),
                  static_cast<double>(config.r2Ohms),
                  static_cast<double>(config.adcMinimumMv));

    executable_example::voltageSensor = executable_example::app.addVoltageSensor(config);
    if (executable_example::voltageSensor == nullptr)
        Serial.println("[example] addVoltageSensor failed; see the runtime log for the cause");

    executable_example::app.setup();
}

void loop()
{
    executable_example::app.handle();

    if (executable_example::voltageSensor == nullptr)
        return;

    const unsigned long nowMs = millis();
    if (nowMs - executable_example::lastReportMs < EXAMPLE_VOLTAGE_LOG_INTERVAL_MS)
        return;

    executable_example::lastReportMs = nowMs;
    executable_example::reportMeasurement();
}
