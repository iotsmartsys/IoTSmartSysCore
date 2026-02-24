#ifdef DHT_SENSOR_ENABLED
#include <Arduino.h>
#include <cmath>

#include "DHTSensor.h"

namespace iotsmartsys::platform::arduino
{
    DHTSensor::DHTSensor(int pin, long readIntervalMs) : pin(pin), readIntervalMs_(readIntervalMs)
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
        if (lastStateReadMillis_ != 0 && (millis() - lastStateReadMillis_) < readIntervalMs_)
        {
            // not time to read yet
            return;
        }

        float temperature = dht->readTemperature();
        float humidity = dht->readHumidity();

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
        return lastTemperatureC_;
    }

    float DHTSensor::getHumidityPercentage()
    {
        return lastHumidity_;
    }

    long DHTSensor::lastStateReadMillis() const
    {
        return lastStateReadMillis_;
    }

} // namespace iotsmartsys::platform::arduino
#endif // DHT_SENSOR_ENABLED