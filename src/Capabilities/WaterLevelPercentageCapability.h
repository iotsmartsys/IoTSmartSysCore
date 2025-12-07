#pragma once

#include <Arduino.h>
#include "Capability.h"
#include "Sensors/Modules/WaterLevelSensor.h"

class WaterLevelPercentCapability : public Capability
{
public:
    WaterLevelPercentCapability(String capability_name, WaterLevelSensor *sensor);

    WaterLevelPercentCapability(WaterLevelSensor *sensor);

    void handle() override;
    float getLevelPercent();

private:
    WaterLevelSensor *sensor;
    float levelPercent;
};
