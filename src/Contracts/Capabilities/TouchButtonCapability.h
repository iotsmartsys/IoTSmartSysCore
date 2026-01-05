#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class TouchButtonCapability : public DebouncedDigitalCapability
    {
    public:
    TouchButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);
    TouchButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);

        void handle() override;

        bool isTouched() const;

    };

} // namespace iotsmartsys::core
