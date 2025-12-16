#pragma once

#include <OneWire.h>
#include <DallasTemperature.h>

#include "Contracts/Sensors/ITemperatureSensor.h"

namespace iotsmartsys::platform::arduino
{
    class DS18B20TemperatureSensor : public core::ITemperatureSensor
    {
    public:
        DS18B20TemperatureSensor(int pin);
        void setup() override;
        float readTemperatureCelsius() override;

    private:
        int pin;
        OneWire *oneWire = nullptr;
        DallasTemperature *sensors = nullptr;
    };
} // namespace iotsmartsys::platform::arduino
