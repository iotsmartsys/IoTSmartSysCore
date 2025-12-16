#pragma once

#include "Contracts/Sensors/IWaterLevelSensor.h"
#include <string>

class TestHeightWaterSensor : public iotsmartsys::core::IWaterLevelSensor
{
public:
    TestHeightWaterSensor() : heightCm(0.0f), percent(0.0f), liters(0.0f) {}
    void setup() override {}
    void handleSensor() override {}
    float getLevelPercent() override { return percent; }
    float getLevelLiters() override { return liters; }
    float getHeightWaterInCm() override { return heightCm; }

    void setHeightCm(float h) { heightCm = h; }
    void setPercent(float p) { percent = p; }
    void setLiters(float l) { liters = l; }

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
        snprintf(buf, sizeof(buf), "%.3f", heightCm);
        return std::string(buf);
    }

private:
    float heightCm;
    float percent;
    float liters;
};
