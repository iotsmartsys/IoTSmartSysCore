#include <Arduino.h>
#include "../src/Core/IoTCore.h"
#include "../src/Utils/Logger.h"

IoTCore *iotCore = new IoTCore();
#define INTERVAL_SENSOR_UPDATE 30 
#define TEMPERATURE_SENSOR_PIN 4
#ifdef DHT_ENABLED
DHT dht(TEMPERATURE_SENSOR_PIN, DHT11);
#endif

void setup()
{
    LOG_BEGIN(115200);
    
    LOG_PRINTLN("Starting IoT Core...");

#ifdef ESP32_S3_AI
    
#endif

#ifdef DS18B20_ENABLED
    iotCore->capabilityBuilder->addTemperatureSensorCapability(TEMPERATURE_SENSOR_PIN, INTERVAL_SENSOR_UPDATE);
#elif DHT_ENABLED
    dht.begin();
    iotCore->capabilityBuilder->addHumiditySensorCapability(&dht, INTERVAL_SENSOR_UPDATE);
    iotCore->capabilityBuilder->addTemperatureSensorCapability(&dht, INTERVAL_SENSOR_UPDATE);
#endif

    iotCore->setup();
}

void loop()
{
    iotCore->handle();
}
