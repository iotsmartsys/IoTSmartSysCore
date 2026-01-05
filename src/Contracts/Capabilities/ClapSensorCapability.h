#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{
    class ClapSensorCapability : public LatchedTriggerCapability
    {
    public:
        ClapSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeSeconds);
    ClapSensorCapability(const char *capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeSeconds);

        void handle() override;

        bool isClapDetected() const;

    private:
        bool isTriggered() const;
    };

} // namespace iotsmartsys::core
