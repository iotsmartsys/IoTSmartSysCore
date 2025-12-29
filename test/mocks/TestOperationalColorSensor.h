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

private:
    std::string _state;
};
