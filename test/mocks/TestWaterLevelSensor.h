#pragma once
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include <string>

// simple test sensor implementation (implements IHardwareAdapter trivial methods)
class TestWaterLevelSensor : public iotsmartsys::core::IWaterLevelSensor
{
public:
    TestWaterLevelSensor() : levelPercent(0.0f), liters(0.0f) {}
    void setup() override {}
    void handleSensor() override {}
    float getLevelPercent() override { return levelPercent; }
    float getLevelLiters() override { return liters; }
    float getHeightWaterInCm() override { return 0.0f; }

    void setLevel(float p) { levelPercent = p; }
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
        snprintf(buf, sizeof(buf), "%.3f", levelPercent);
        return std::string(buf);
    }

private:
    float levelPercent;
    float liters;
};