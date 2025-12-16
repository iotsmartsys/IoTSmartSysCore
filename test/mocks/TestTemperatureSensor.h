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

    // IHardwareAdapter trivial implementations for test mock
    bool applyCommand(const iotsmartsys::core::IHardwareCommand &command) override
    {
        (void)command;
        return false;
    }

    bool applyCommand(const std::string &value) override
    {
        (void)value;
        return false;
    }

    std::string getState() override
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", tempCelsius);
        return std::string(buf);
    }

private:
    float tempCelsius;
};
