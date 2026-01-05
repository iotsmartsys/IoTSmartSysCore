#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{

    class PirSensorCapability : public LatchedTriggerCapability
    {
    public:
        PirSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTime);
        PirSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTime);

        void handle() override;

        bool isPresenceDetected() const;

    private:
        bool isTriggered() const;
    };
} // namespace iotsmartsys::core
