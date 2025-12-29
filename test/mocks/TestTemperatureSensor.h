#pragma once

#include "Contracts/Sensors/ITemperatureSensor.h"
#include <string>

class TestTemperatureSensor : public iotsmartsys::core::ITemperatureSensor
{
public:
    TestTemperatureSensor() : tempCelsius(0.0f) {}
    void setup() override {}
    float readTemperatureCelsius() override { return tempCelsius; }

    void setTemperature(float t) { tempCelsius = t; }

private:
    float tempCelsius;
};
