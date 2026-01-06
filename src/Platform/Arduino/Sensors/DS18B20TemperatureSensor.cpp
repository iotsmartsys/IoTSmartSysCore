#include <Arduino.h>
#include <cmath>

#include "DS18B20TemperatureSensor.h"

namespace iotsmartsys::platform::arduino
{
    DS18B20TemperatureSensor::DS18B20TemperatureSensor(int pin) : pin(pin)
    {
    }

    void DS18B20TemperatureSensor::setup()
    {
        oneWire = new OneWire(pin);
        sensors = new DallasTemperature(oneWire);
        sensors->begin();
    }

    void DS18B20TemperatureSensor::handle()
    {
        if (!sensors)
        {
            return;
        }

        float temperature = readTemperatureCelsius();

        if (!hasReading_ || fabs(temperature - lastTemperatureC_) > 0.01f)
        {
            lastTemperatureC_ = temperature;
            hasReading_ = true;
            lastStateReadMillis_ = millis();
        }
    }

    float DS18B20TemperatureSensor::readTemperatureCelsius()
    {
        if (!sensors)
        {
            return -180.0f;
        }

        sensors->requestTemperatures();
        return sensors->getTempCByIndex(0);
    }

    long DS18B20TemperatureSensor::lastStateReadMillis() const
    {
        return lastStateReadMillis_;
    }

} // namespace iotsmartsys::platform::arduino
