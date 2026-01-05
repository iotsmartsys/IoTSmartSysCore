#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IGlpMeter.h"
#include <string>

namespace iotsmartsys::core
{
    class GlpMeterKgCapability : public ICapability
    {
    public:
        explicit GlpMeterKgCapability(IGlpMeter &meter, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

        float getKg() const;

    private:
        IGlpMeter &meter;
        float lastKg{0.0f};
        unsigned long lastCheckMillis{0};
    };

} // namespace iotsmartsys::core
