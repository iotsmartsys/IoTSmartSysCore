#pragma once

#include "IInputCapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{

    class PirSensorCapability : public IInputCapability
    {
    public:
        PirSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTime);
        PirSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTime);

        void setup() override;
        void handle() override;

        bool isPresenceDetected() const;

    private:
        long lastTimePresenceDetected;
        bool presenceDetected;
        bool lastState;
        int timeTolerance;
        bool pirState;

        bool isTriggered() const;
        long getTimeSinceLastPresenceDetected() const;
    };
} // namespace iotsmartsys::core