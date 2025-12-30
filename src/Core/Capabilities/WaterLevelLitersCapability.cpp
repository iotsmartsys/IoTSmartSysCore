#include "Contracts/Capabilities/WaterLevelLitersCapability.h"
#include <sstream>
#include <iomanip>

namespace iotsmartsys::core
{
    WaterLevelLitersCapability::WaterLevelLitersCapability(IWaterLevelSensor &sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, WATER_LEVEL_LITERS_SENSOR_TYPE, "0"), sensor(sensor), levelLiters(0.0f), lastLevelLiters(0.0f), lastCheckMillis(0)
    {
    }

    void WaterLevelLitersCapability::setup()
    {
        sensor.setup();
    }

    void WaterLevelLitersCapability::handle()
    {
        unsigned long now = timeProvider.nowMs();
        if (now - lastCheckMillis >= 1000 || lastLevelLiters == 0.0)
        {
            sensor.handle();
            // Check every second
            lastCheckMillis = now;

            float actualLevel = sensor.getLevelLiters();

            if (lastLevelLiters != actualLevel && actualLevel >= 0.0)
            {
                lastLevelLiters = actualLevel;
                levelLiters = actualLevel;
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << levelLiters;
                updateState(ss.str());
                lastCheckMillis = now;
            }
        }
    }

    float WaterLevelLitersCapability::getLevelLiters()
    {
        return levelLiters;
    }

} // namespace iotsmartsys::core