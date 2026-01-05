#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class PushButtonCapability : public DebouncedDigitalCapability
    {
    public:
        // toleranceTime in milliseconds for debouncing / event grouping
        PushButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);
        PushButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);

        void handle() override;

        bool isPressed() const;
    };

} // namespace iotsmartsys::core
