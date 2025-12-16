#include "Contracts/Capabilities/GlpSensorCapability.h"

namespace iotsmartsys::core
{
    GlpSensorCapability::GlpSensorCapability(IGlpSensor *sensor)
        : ICapability(nullptr, GLP_SENSOR_TYPE, GLP_SENSOR_LEVEL_NONE), sensor(sensor), levelPercent(0.0f), lastLevel(), lastCheckMillis(0)
    {
    }

    void GlpSensorCapability::setup()
    {
        ICapability::setup();
        if (sensor)
            sensor->setup();
    }

    void GlpSensorCapability::handle()
    {
        if (!sensor)
            return;

        unsigned long now = timeProvider.nowMs();
        if (now - lastCheckMillis >= 1000 || lastLevel.empty())
        {
            lastCheckMillis = now;
            sensor->handleSensor();

            float percent = sensor->getLevelPercent();
            std::string levelState = sensor->getLevelString();

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
        if (!sensor)
            return false;
        return sensor->isDetected();
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
