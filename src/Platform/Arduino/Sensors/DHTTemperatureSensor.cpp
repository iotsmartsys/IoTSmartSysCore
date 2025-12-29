#include <Arduino.h>

#include "DHTTemperatureSensor.h"

namespace iotsmartsys::platform::arduino
{
    DHTTemperatureSensor::DHTTemperatureSensor(int pin) : pin(pin)
    {
        // Default to DHT11; using the wrong sensor type causes NaN reads.
        dht = new DHT(pin, DHT11);
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
