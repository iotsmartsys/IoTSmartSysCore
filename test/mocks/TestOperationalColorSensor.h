#pragma once

#include "Contracts/Sensors/IColorSensor.h"
#include <string>

class TestOperationalColorSensor : public iotsmartsys::core::IColorSensor
{
public:
    TestOperationalColorSensor() : _state("green") {}
    ~TestOperationalColorSensor() override = default;

    void setup() override {}
    void handle() override {}

    std::string getStateString() const override { return _state; }

    void setState(const std::string &s) { _state = s; }

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
        return _state;
    }

private:
    std::string _state;
};
