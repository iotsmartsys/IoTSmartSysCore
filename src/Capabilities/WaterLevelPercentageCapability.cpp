#include "WaterLevelPercentageCapability.h"

WaterLevelPercentCapability::WaterLevelPercentCapability(String capability_name, WaterLevelSensor *sensor)
    : Capability(capability_name, WATER_LEVEL_PERCENT_SENSOR_TYPE, "0")
{
    this->sensor = sensor;
}

WaterLevelPercentCapability::WaterLevelPercentCapability(WaterLevelSensor *sensor)
    : Capability("", WATER_LEVEL_PERCENT_SENSOR_TYPE, "0")
{
    this->sensor = sensor;
}

void WaterLevelPercentCapability::handle()
{
    float actualLevelPercent = sensor->getLevelPercent();
    if (abs(actualLevelPercent - levelPercent) >= 1.0)
    {
        levelPercent = actualLevelPercent;
        updateState(String(levelPercent));
    }
}

float WaterLevelPercentCapability::getLevelPercent()
{
    return levelPercent;
}
