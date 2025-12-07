#include "Arduino.h"
#include "IoTSmartSysCore.h"
#ifdef DHT_ENABLED
#include <DHT.h>
#endif

#if defined(DHT_ENABLED)
#ifdef ESP32
#define DHT_PIN 23
#elif defined(ESP8266)
#define DHT_PIN 2
#endif
DHT dht(DHT_PIN, DHT11);
#endif

#ifdef ESP32
#define SDA_PIN 21
#define SCL_PIN 22
#elif defined(ESP8266)
#define SDA_PIN 4
#define SCL_PIN 5
#endif
IoTCore *iotCore = new IoTCore();

void setup()
{
#ifdef BH1750_ENABLED
    iotCore->addLuminositySensorCapability(SDA_PIN, SCL_PIN);
#endif
#ifdef DHT_ENABLED
    iotCore->addTemperatureSensorCapability(&dht, 24);
    iotCore->addHumiditySensorCapability(&dht, 25);
#endif

    iotCore->setup();
}

void loop()
{
    iotCore->handle();
}
