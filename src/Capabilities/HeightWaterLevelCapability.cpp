#include "HeightWaterLevelCapability.h"

HeightWaterLevelCapability::HeightWaterLevelCapability(String capability_name, WaterLevelSensor *sensor)
    : Capability(capability_name, HEIGHT_WATER_LEVEL_SENSOR_TYPE, "0"), sensor(sensor), actualHeight(0.0), lastHeight(0.0), lastTime(0.0), lastCheckMillis(0)
{
}

void HeightWaterLevelCapability::handle()
{
    actualHeight = sensor->getHeightWaterInCm();
    unsigned long now = millis();
    if (lastTime == 0)
    {
        lastTime = now;
    }
    if (now - lastTime < 1000)
    {
        return;
    }
    lastTime = now;
    if (lastHeight != actualHeight)
    {
        lastHeight = actualHeight;
        updateState(String(actualHeight));
    }
}

float HeightWaterLevelCapability::getHeightWaterInCm()
{
    return actualHeight;
}
