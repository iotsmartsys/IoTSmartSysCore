#include "HumiditySensorCapability.h"
#include "Utils/Logger.h"
#ifdef DHT_ENABLED
#include <Arduino.h>

HumiditySensorCapability::HumiditySensorCapability(DHT *dht, unsigned long intervalMinutes)
    : Capability("", HUMIDITY_SENSOR_TYPE, "0"), dht(dht), humidity(0), lastReadTime(0), readIntervalMs(intervalMinutes * 60000)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Nada a fazer com o estado: " + *state);
                });
}

HumiditySensorCapability::HumiditySensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), dht(nullptr), humidity(0), lastReadTime(0), readIntervalMs(60000)
{
}

void HumiditySensorCapability::handle()
{
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readIntervalMs && humidity > 0)
    {
        return;
    }
    lastReadTime = currentTime;

    float current_humidity = dht->readHumidity();
    LOG_PRINTLN("A umidade é: " + String(current_humidity) + "%");
    if (current_humidity != humidity && !isnan(current_humidity) && current_humidity > 0)
    {
        humidity = current_humidity;
        updateState(String(humidity));
    }
}

float HumiditySensorCapability::getHumidity()
{
    return humidity;
}

void HumiditySensorCapability::setReadIntervalMs(unsigned long intervalMs)
{
    this->readIntervalMs = intervalMs;
}

#endif
