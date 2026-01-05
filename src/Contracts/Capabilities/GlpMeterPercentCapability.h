#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IGlpMeter.h"
#include <string>

namespace iotsmartsys::core
{
    class GlpMeterPercentCapability : public ICapability
    {
    public:
        explicit GlpMeterPercentCapability(IGlpMeter &meter, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

        float getPercent() const;

    private:
        IGlpMeter &meter;
        float lastPercent{0.0f};
        std::string lastState;
        unsigned long lastCheckMillis{0};
    };

} // namespace iotsmartsys::core
