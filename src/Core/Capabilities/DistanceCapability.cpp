#include "Contracts/Capabilities/DistanceCapability.h"

namespace iotsmartsys::core
{
    DistanceCapability::DistanceCapability(IDistanceSensor &sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, DISTANCE_SENSOR_TYPE, "0"), sensor(sensor), distanceCm(0.0f), lastDistanceCm(0.0f), lastCheckMillis(0)
    {
    }

    void DistanceCapability::handle()
    {
        unsigned long now = timeProvider.nowMs();
        // Check every second
        if (now - lastCheckMillis >= 1000 || lastDistanceCm == 0.0f)
        {
            lastCheckMillis = now;

            sensor.handle();

            float actualDistance = sensor.getActualDistanceCm();
            if (lastDistanceCm != actualDistance)
            {
                lastDistanceCm = actualDistance;
                distanceCm = actualDistance;
                updateState(std::to_string(distanceCm));
                lastCheckMillis = now;
            }
        }
    }

    float DistanceCapability::getDistanceCm() const
    {
        return distanceCm;
    }

} // namespace iotsmartsys::core
