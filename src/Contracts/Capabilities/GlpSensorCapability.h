#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IGlpSensor.h"

namespace iotsmartsys::core
{
    class GlpSensorCapability : public ICapability
    {
    public:
        GlpSensorCapability(IGlpSensor *sensor, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

        bool isDetected() const;
        float getLevelPercent() const;
        std::string getLevelString() const;

    private:
        IGlpSensor *sensor;
        float levelPercent{0.0f};
        std::string lastLevel;
        unsigned long lastCheckMillis{0};
    };

} // namespace iotsmartsys::core
