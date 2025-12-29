#include "Contracts/Capabilities/HeightWaterLevelCapability.h"

namespace iotsmartsys::core
{
    HeightWaterLevelCapability::HeightWaterLevelCapability(IWaterLevelSensor &sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, HEIGHT_WATER_LEVEL_SENSOR_TYPE, "0"), sensor(sensor), levelCm(0.0f), lastLevelCm(0.0f), lastCheckMillis(0)
    {
    }

    void HeightWaterLevelCapability::handle()
    {
        unsigned long now = timeProvider.nowMs();
        // Check every second
        if (now - lastCheckMillis >= 1000 || lastLevelCm == 0.0f)
        {
            lastCheckMillis = now;

            sensor.handle();

            float actualLevel = sensor.getHeightWaterInCm();
            if (lastLevelCm != actualLevel)
            {
                lastLevelCm = actualLevel;
                levelCm = actualLevel;
                updateState(std::to_string(levelCm));
                lastCheckMillis = now;
            }
        }
    }

    float HeightWaterLevelCapability::getHeightWaterInCm() const
    {
        return levelCm;
    }

} // namespace iotsmartsys::core
