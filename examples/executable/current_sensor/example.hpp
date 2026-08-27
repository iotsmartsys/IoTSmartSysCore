#pragma once

#include <Arduino.h>
#include <cmath>
#include "SmartSysApp.h"

#ifndef ITS_MCB01_J4_EXT_ADC
#error "current_sensor requires the MCB R1 pinout symbol ITS_MCB01_J4_EXT_ADC"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "current_sensor requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

#ifndef EXAMPLE_CURRENT_SENSOR_ID
#error "current_sensor requires EXAMPLE_CURRENT_SENSOR_ID to be defined"
#endif

#ifndef EXAMPLE_CURRENT_LOG_INTERVAL_MS
#error "current_sensor requires EXAMPLE_CURRENT_LOG_INTERVAL_MS to be defined"
#endif

#if (defined(EXAMPLE_CURRENT_SENSOR_PROFILE_3V3) + defined(EXAMPLE_CURRENT_SENSOR_PROFILE_5V)) != 1
#error "current_sensor requires exactly one of EXAMPLE_CURRENT_SENSOR_PROFILE_3V3 or EXAMPLE_CURRENT_SENSOR_PROFILE_5V"
#endif

namespace executable_example
{
    using iotsmartsys::core::CurrentMeasurement;
    using iotsmartsys::core::CurrentSensorCapability;
    using iotsmartsys::core::CurrentSensorConfig;

    static iotsmartsys::SmartSysApp app;
    static CurrentSensorCapability *currentSensor = nullptr;
    static unsigned long lastReportMs = 0;

    static const char *profileId()
    {
#if defined(EXAMPLE_CURRENT_SENSOR_PROFILE_3V3)
        return "ACS712_30A_3V3";
#else
        return "ACS712_30A_5V";
#endif
    }

    static CurrentSensorConfig buildConfig()
    {
#if defined(EXAMPLE_CURRENT_SENSOR_PROFILE_3V3)
        return CurrentSensorConfig::ACS712_30A_3V3(EXAMPLE_CURRENT_SENSOR_ID, ITS_MCB01_J4_EXT_ADC);
#else
        return CurrentSensorConfig::ACS712_30A_5V(EXAMPLE_CURRENT_SENSOR_ID, ITS_MCB01_J4_EXT_ADC);
#endif
    }

    // Apresenta a última medição estável publicada pela capability. Nenhuma
    // aquisição, conversão ou qualificação é feita aqui.
    static void reportMeasurement()
    {
        const CurrentMeasurement &measurement = currentSensor->currentMeasurement();
        const char *measurementStatus = iotsmartsys::core::toString(measurement.measurementStatus);
        const char *supplyStatus = iotsmartsys::core::toString(measurement.supplyStatus);

        if (measurement.currentA.has_value())
        {
            float currentA = measurement.currentA.value();
            if (std::fabs(currentA) < 0.0005f)
                currentA = 0.0f;
            Serial.printf("[example] current=%.3f A measurementStatus=%s supplyStatus=%s\n",
                          currentA, measurementStatus, supplyStatus);
        }
        else
        {
            Serial.printf("[example] current=<none> measurementStatus=%s supplyStatus=%s\n",
                          measurementStatus, supplyStatus);
        }
    }

    // Estímulo local do operador: 'c' agenda a recalibração de zero para o
    // próximo handle(). Exige corrente zero garantida externamente e
    // alimentação estável.
    static void handleOperatorInput()
    {
        while (Serial.available() > 0)
        {
            const int command = Serial.read();
            if (command == 'c' || command == 'C')
            {
                Serial.println("[example] zero calibration requested");
                currentSensor->requestZeroCalibration();
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);

    const iotsmartsys::core::CurrentSensorConfig config = executable_example::buildConfig();
    Serial.printf("[example] id=current_sensor board=%s signal_pin=%d profile=%s capability=%s warmup_ms=%lu\n",
                  EXAMPLE_BOARD_ID, ITS_MCB01_J4_EXT_ADC, executable_example::profileId(),
                  config.id.c_str(), static_cast<unsigned long>(config.startupWarmupMs));
    Serial.println("[example] supply monitoring disabled: supplyStatus stays NOT_MONITORED");
    Serial.println("[example] send 'c' to request a zero calibration at zero current");

    executable_example::currentSensor = executable_example::app.addCurrentSensor(config);
    if (executable_example::currentSensor == nullptr)
        Serial.println("[example] addCurrentSensor failed; see the runtime log for the cause");

    executable_example::app.setup();
}

void loop()
{
    executable_example::app.handle();

    if (executable_example::currentSensor == nullptr)
        return;

    executable_example::handleOperatorInput();

    const unsigned long nowMs = millis();
    if (nowMs - executable_example::lastReportMs < EXAMPLE_CURRENT_LOG_INTERVAL_MS)
        return;

    executable_example::lastReportMs = nowMs;
    executable_example::reportMeasurement();
}
