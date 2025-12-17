#include "Contracts/Capabilities/WaterLevelLitersCapability.h"

namespace iotsmartsys::core
{
    WaterLevelLitersCapability::WaterLevelLitersCapability(IWaterLevelSensor *sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, WATER_LEVEL_LITERS_SENSOR_TYPE, "0"), sensor(sensor), levelLiters(0.0f), lastLevelLiters(0.0f), lastCheckMillis(0)
    {
    }

    void WaterLevelLitersCapability::handle()
    {
        if (!sensor)
            return;

        unsigned long now = timeProvider.nowMs();
        // Check every second
        if (now - lastCheckMillis >= 1000 || lastLevelLiters == 0.0)
        {
            lastCheckMillis = now;

            sensor->handle();

            float actualLevel = sensor->getLevelLiters();
            if (lastLevelLiters != actualLevel)
            {
                lastLevelLiters = actualLevel;
                levelLiters = actualLevel;
                updateState(std::to_string(levelLiters));
                lastCheckMillis = now;
            }
        }
    }

    float WaterLevelLitersCapability::getLevelLiters()
    {
        return levelLiters;
    }

} // namespace iotsmartsys::core