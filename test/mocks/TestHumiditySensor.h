#pragma once

#include "Contracts/Sensors/IHumiditySensor.h"
#include <string>

class TestHumiditySensor : public iotsmartsys::core::IHumiditySensor
{
public:
    TestHumiditySensor() : humidityPercent(0.0f) {}
    void setup() override {}
    float getHumidityPercentage() override { return humidityPercent; }

    void setHumidity(float h) { humidityPercent = h; }

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
        snprintf(buf, sizeof(buf), "%.3f", humidityPercent);
        return std::string(buf);
    }

private:
    float humidityPercent;
};
