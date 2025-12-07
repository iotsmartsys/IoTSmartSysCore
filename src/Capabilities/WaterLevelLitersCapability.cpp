#include "WaterLevelLitersCapability.h"

WaterLevelLitersCapability::WaterLevelLitersCapability(String capability_name, WaterLevelSensor *sensor)
    : Capability(capability_name, WATER_LEVEL_LITERS_SENSOR_TYPE, "0")
{
    this->sensor = sensor;
}

void WaterLevelLitersCapability::handle()
{
    unsigned long now = millis();
    float actualLevel = sensor->getLevelLiters();

    float diff = 0;
    if (lastLevelLiters > actualLevel)
    {
        diff = lastLevelLiters - actualLevel;
    }
    else
    {
        diff = actualLevel - lastLevelLiters;
    }

    if (diff > 5.0 && (now - lastCheckMillis < 15000))
    {
        return;
    }

    if (lastLevelLiters != actualLevel)
    {
        lastLevelLiters = actualLevel;
        levelLiters = actualLevel;
        updateState(String(levelLiters));
        lastCheckMillis = now;
    }
}

float WaterLevelLitersCapability::getLevelLiters()
{
    return levelLiters;
}
