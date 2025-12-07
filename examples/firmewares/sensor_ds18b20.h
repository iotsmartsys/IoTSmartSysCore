#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include <math.h>

#if defined(ESP32)
#define TEMPERATURE_SENSOR 23
#elif defined(ESP8266)
#define TEMPERATURE_SENSOR D2
#endif

IoTCore *iotCore = new IoTCore();

void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");

#if defined(DS18B20_ENABLED)
    iotCore->addTemperatureSensorCapability(TEMPERATURE_SENSOR);
#endif
#ifdef LED_BUILTIN
    iotCore->configureLEDControl(LED_BUILTIN, DigitalLogic::INVERTED);
#endif
    iotCore->setup();
}

void loop()
{
    iotCore->handle();
}
