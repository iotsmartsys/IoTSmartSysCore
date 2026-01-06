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
        void handle() override;
        float readTemperatureCelsius() override;
        long lastStateReadMillis() const override;

    private:
        int pin;
        OneWire *oneWire = nullptr;
        DallasTemperature *sensors = nullptr;
        float lastTemperatureC_{0.0f};
        long lastStateReadMillis_{0};
        bool hasReading_{false};
    };
} // namespace iotsmartsys::platform::arduino
