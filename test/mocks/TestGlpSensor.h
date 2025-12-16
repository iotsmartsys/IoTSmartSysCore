#pragma once

#include "Contracts/Sensors/IGlpSensor.h"
#include <string>

class TestGlpSensor : public iotsmartsys::core::IGlpSensor
{
public:
    TestGlpSensor() : levelPercent(0.0f), detected(false), levelState("none") {}
    void setup() override {}
    void handleSensor() override {}
    float getLevelPercent() override { return levelPercent; }
    bool isDetected() override { return detected; }
    std::string getLevelString() override { return levelState; }

    void setLevelPercent(float p) { levelPercent = p; }
    void setDetected(bool d) { detected = d; }
    void setLevelState(const std::string &s) { levelState = s; }

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
        return levelState;
    }

private:
    float levelPercent;
    bool detected;
    std::string levelState;
};
