#include "Contracts/Capabilities/GlpSensorCapability.h"

namespace iotsmartsys::core
{
    GlpSensorCapability::GlpSensorCapability(IGlpSensor &sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, GLP_SENSOR_TYPE, GlpSensorLevelStrings::UNDETECTED), sensor(sensor), levelPercent(0.0f), lastLevel(), lastCheckMillis(0)
    {
    }

    void GlpSensorCapability::setup()
    {
        ICapability::setup();
        sensor.setup();
    }

    void GlpSensorCapability::handle()
    {
        unsigned long now = timeProvider.nowMs();
        if (now - lastCheckMillis >= 1000 || lastLevel.empty())
        {
            lastCheckMillis = now;
            sensor.handle();

            float percent = sensor.getLevelPercent();
            std::string levelState = sensor.getLevelString();

            if (levelState != lastLevel)
            {
                lastLevel = levelState;
                levelPercent = percent;
                updateState(levelState);
            }
        }
    }

    bool GlpSensorCapability::isDetected() const
    {
        return sensor.isDetected();
    }

    float GlpSensorCapability::getLevelPercent() const
    {
        return levelPercent;
    }

    std::string GlpSensorCapability::getLevelString() const
    {
        return lastLevel;
    }

} // namespace iotsmartsys::core
