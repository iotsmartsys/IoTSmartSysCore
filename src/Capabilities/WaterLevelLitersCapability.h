#pragma once

#include <Arduino.h>
#include "Capability.h"
#include "Infra/Sensors/Modules/WaterLevelSensor.h"

class WaterLevelLitersCapability : public Capability
{
public:
    WaterLevelLitersCapability(String capability_name, WaterLevelSensor *sensor);

    void handle() override;
    float getLevelLiters();

private:
    WaterLevelSensor *sensor;
    float levelLiters;
    float lastLevelLiters = 0.0;
    unsigned long lastCheckMillis = 0;
};
