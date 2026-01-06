#include <Arduino.h>
#include <cmath>

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

    void DHTSensor::handle()
    {
        float temperature = readTemperatureCelsius();
        float humidity = getHumidityPercentage();

        // consider it a state change when either value shifts
        if (!hasReading_ || fabs(temperature - lastTemperatureC_) > 0.01f || fabs(humidity - lastHumidity_) > 0.01f)
        {
            lastTemperatureC_ = temperature;
            lastHumidity_ = humidity;
            hasReading_ = true;
            lastStateReadMillis_ = millis();
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

    long DHTSensor::lastStateReadMillis() const
    {
        return lastStateReadMillis_;
    }

} // namespace iotsmartsys::platform::arduino
