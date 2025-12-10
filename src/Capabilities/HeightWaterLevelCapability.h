#pragma once

#include <Arduino.h>
#include "Capability.h"
#include "Infra/Sensors/Modules/WaterLevelSensor.h"

class HeightWaterLevelCapability : public Capability
{
public:
    HeightWaterLevelCapability(String capability_name, WaterLevelSensor *sensor);

    void handle() override;

    float getHeightWaterInCm();

private:
    WaterLevelSensor *sensor;
    float actualHeight;
    float lastHeight;
    float lastTime;
    unsigned long lastCheckMillis;
};
