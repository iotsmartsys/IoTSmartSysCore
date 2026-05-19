#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IDistanceSensor.h"

namespace iotsmartsys::core
{
    class DistanceCapability : public ICapability
    {
    public:
        DistanceCapability(IDistanceSensor &sensor, ICapabilityEventSink *event_sink);

        void handle() override;
        float getDistanceCm() const;

    private:
        IDistanceSensor &sensor;
        float distanceCm;
        float lastDistanceCm = 0.0f;
        unsigned long lastCheckMillis = 0;
    };

} // namespace iotsmartsys::core
