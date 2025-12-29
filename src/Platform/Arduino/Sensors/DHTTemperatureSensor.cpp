#include <Arduino.h>

#include "DHTTemperatureSensor.h"

namespace iotsmartsys::platform::arduino
{
    DHTTemperatureSensor::DHTTemperatureSensor(int pin) : pin(pin)
    {
        dht = new DHT(pin, DHT22);
    }

    void DHTTemperatureSensor::setup()
    {
        if (dht)
        {
            dht->begin();
        }
    }

    float DHTTemperatureSensor::readTemperatureCelsius()
    {
        if (dht)
        {
            return dht->readTemperature();
        }
        return -180.0f;
    }

    float DHTTemperatureSensor::getHumidityPercentage()
    {
        if (dht)
        {
            return dht->readHumidity();
        }
        return -180.0f;
    }

} // namespace iotsmartsys::platform::arduino