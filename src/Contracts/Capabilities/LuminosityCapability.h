// Core/LuminosityCapability.h
#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"
#include "Contracts/Sensors/ILuminositySensor.h"

namespace iotsmartsys::core
{
    class LuminosityCapability : public PollingFloatCapability
    {
    public:
        LuminosityCapability(const std::string &name,
                             ILuminositySensor &sensor,
                             ICapabilityEventSink *event_sink,
                             float variationTolerance,
                             float readIntervalMs);

        LuminosityCapability(ILuminositySensor &sensor,
                             ICapabilityEventSink *event_sink,
                             float variationTolerance,
                             float readIntervalMs);

        void setup() override;
        void handle() override;
        float getLux();

    protected:
    private:
        ILuminositySensor &_sensor;
    };

} // namespace iotsmartsys::core
