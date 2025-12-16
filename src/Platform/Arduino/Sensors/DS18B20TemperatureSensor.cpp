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

    float DS18B20TemperatureSensor::readTemperatureCelsius()
    {
        sensors->requestTemperatures();
        return sensors->getTempCByIndex(0);
    }

} // namespace iotsmartsys::platform::arduino
