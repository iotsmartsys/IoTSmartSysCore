#pragma once

#include <Arduino.h>
#include <memory>
#include "SmartSysApp.h"

#ifndef DHT_SENSOR_ENABLED
#error "environment_dht requires DHT_SENSOR_ENABLED"
#endif

#ifndef EXAMPLE_DHT_PIN
#error "environment_dht requires EXAMPLE_DHT_PIN to be defined by the PlatformIO environment"
#endif

#ifndef EXAMPLE_DHT_READ_INTERVAL_MS
#error "environment_dht requires EXAMPLE_DHT_READ_INTERVAL_MS to be defined"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "environment_dht requires EXAMPLE_BOARD_ID to be defined by the board profile"
#endif

namespace executable_example
{
    using iotsmartsys::core::ServiceManager;
    using iotsmartsys::infra::factories::SensorFactory;
    using iotsmartsys::platform::arduino::DHTSensor;

    static iotsmartsys::SmartSysApp app;
    static SensorFactory sensorFactory{ServiceManager::init().logger()};
    static std::unique_ptr<DHTSensor> dhtSensor;
}

void setup()
{
    Serial.begin(115200);
    Serial.printf("[example] id=environment_dht board=%s dht_model=DHT11 dht_pin=%d read_interval_ms=%ld\n",
                  EXAMPLE_BOARD_ID, EXAMPLE_DHT_PIN,
                  static_cast<long>(EXAMPLE_DHT_READ_INTERVAL_MS));

    executable_example::dhtSensor = executable_example::sensorFactory.createDHTSensor(
        EXAMPLE_DHT_PIN, EXAMPLE_DHT_READ_INTERVAL_MS);

    iotsmartsys::app::TemperatureSensorConfig temperatureConfig;
    temperatureConfig.capability_name = "environment_temperature";
    temperatureConfig.sensor = executable_example::dhtSensor.get();
    temperatureConfig.readIntervalMs = EXAMPLE_DHT_READ_INTERVAL_MS;
    executable_example::app.addTemperatureSensorCapability(temperatureConfig);

    iotsmartsys::app::HumiditySensorConfig humidityConfig;
    humidityConfig.capability_name = "environment_humidity";
    humidityConfig.sensor = executable_example::dhtSensor.get();
    humidityConfig.readIntervalMs = EXAMPLE_DHT_READ_INTERVAL_MS;
    executable_example::app.addHumiditySensorCapability(humidityConfig);

    executable_example::app.setup();
}

void loop()
{
    executable_example::dhtSensor->handle();
    executable_example::app.handle();
}
