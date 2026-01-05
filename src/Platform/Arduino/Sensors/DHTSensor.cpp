#include <Arduino.h>

#include "DHTSensor.h"

namespace iotsmartsys::platform::arduino
{
    DHTSensor::DHTSensor(int pin) : pin(pin)
    {
        // Default to DHT11; using the wrong sensor type causes NaN reads.
        dht = new DHT(pin, DHT11);
    }

    void DHTSensor::setup()
    {
        if (dht)
        {
            dht->begin();
        }
    }

    float DHTSensor::readTemperatureCelsius()
    {
        if (dht)
        {
            return dht->readTemperature();
        }
        return -180.0f;
    }

    float DHTSensor::getHumidityPercentage()
    {
        if (dht)
        {
            return dht->readHumidity();
        }
        return -180.0f;
    }

} // namespace iotsmartsys::platform::arduino
