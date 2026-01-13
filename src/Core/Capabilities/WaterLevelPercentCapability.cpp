#include "Contracts/Capabilities/WaterLevelPercentCapability.h"
#include <cstdio>

namespace iotsmartsys::core
{
    WaterLevelPercentCapability::WaterLevelPercentCapability(IWaterLevelSensor &sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, WATER_LEVEL_PERCENT_SENSOR_TYPE, "0"), sensor(sensor), lastPercent(0.0f)
    {
    }

    void WaterLevelPercentCapability::setup()
    {
    }

    void WaterLevelPercentCapability::handle()
    {
        sensor.handle();

        float current = sensor.getLevelPercent();
        if (current != lastPercent)
        {
            lastPercent = current;
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", current);
            updateState(buf);
        }
    }

    float WaterLevelPercentCapability::getLevelPercent() const
    {
        return lastPercent;
    }

} // namespace iotsmartsys::core
