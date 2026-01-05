#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"
#include "Contracts/Sensors/IHumiditySensor.h"

namespace iotsmartsys::core
{
    class HumiditySensorCapability : public PollingFloatCapability
    {
    public:
        HumiditySensorCapability(IHumiditySensor &sensor, ICapabilityEventSink *event_sink);
        HumiditySensorCapability(std::string capability_name, IHumiditySensor &sensor, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;
        float getHumidity() const;

    private:
        IHumiditySensor &sensor;

        bool isValidHumidity(float hum) const;
    };

} // namespace iotsmartsys::core
