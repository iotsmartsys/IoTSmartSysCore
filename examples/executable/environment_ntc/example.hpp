#pragma once

#include <Arduino.h>
#include <memory>
#include "SmartSysApp.h"

#ifndef ITS_MCB01_J4_EXT_ADC
#error "environment_ntc requires the MCB R1 pinout symbol ITS_MCB01_J4_EXT_ADC"
#endif

#ifndef EXAMPLE_NTC_READ_INTERVAL_MS
#error "environment_ntc requires EXAMPLE_NTC_READ_INTERVAL_MS to be defined"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "environment_ntc requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

namespace executable_example
{
    using iotsmartsys::core::ServiceManager;
    using iotsmartsys::infra::factories::SensorFactory;
    using iotsmartsys::platform::arduino::NtcTemperatureSensor;
    using iotsmartsys::platform::arduino::NtcTemperatureSensorConfig;

    static iotsmartsys::SmartSysApp app;
    static SensorFactory sensorFactory{ServiceManager::init().logger()};
    static std::unique_ptr<NtcTemperatureSensor> ntcSensor;
}

void setup()
{
    Serial.begin(115200);

    const auto ntcConfig =
        executable_example::NtcTemperatureSensorConfig::MF52_103_B3950(
            ITS_MCB01_J4_EXT_ADC);
    Serial.printf(
        "[example] id=environment_ntc board=%s adc_pin=%d profile=MF52_103_B3950 "
        "R0=%.0f beta=%.0f T0=%.1f series=%.0f supply=%.2fV adc_reference=%.2fV "
        "samples=%u read_interval_ms=%ld\n",
        EXAMPLE_BOARD_ID,
        ntcConfig.adcPin,
        static_cast<double>(ntcConfig.nominalResistanceOhms),
        static_cast<double>(ntcConfig.betaK),
        static_cast<double>(ntcConfig.referenceTemperatureC),
        static_cast<double>(ntcConfig.seriesResistanceOhms),
        static_cast<double>(ntcConfig.supplyVoltageV),
        static_cast<double>(ntcConfig.adcReferenceVoltageV),
        static_cast<unsigned>(executable_example::NtcTemperatureSensor::SAMPLES_PER_READING),
        static_cast<long>(EXAMPLE_NTC_READ_INTERVAL_MS));

    executable_example::ntcSensor =
        executable_example::sensorFactory.createNtcTemperatureSensor(ntcConfig);
    if (executable_example::ntcSensor)
    {
        iotsmartsys::app::TemperatureSensorConfig temperatureConfig;
        temperatureConfig.capability_name = "environment_temperature";
        temperatureConfig.sensor = executable_example::ntcSensor.get();
        temperatureConfig.readIntervalMs = EXAMPLE_NTC_READ_INTERVAL_MS;
        executable_example::app.addTemperatureSensorCapability(temperatureConfig);
    }
    else
    {
        Serial.println("[example] createNtcTemperatureSensor failed; see runtime log");
    }

    executable_example::app.setup();
}

void loop()
{
    executable_example::app.handle();
}
