#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IGlpMeter.h"
#include <string>

namespace iotsmartsys::core
{
    class GlpMeterCapability : public ICapability
    {
    public:
        explicit GlpMeterCapability(IGlpMeter &meter, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

        float getKg() const;
        float getPercent() const;

    private:
        IGlpMeter &meter;
        float lastKg{0.0f};
        float lastPercent{0.0f};
        std::string lastState;
        unsigned long lastCheckMillis{0};
    };

} // namespace iotsmartsys::core
