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


private:
    float humidityPercent;
};
