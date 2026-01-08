#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class SwitchPlugCapability : public BinaryCommandCapability
    {
    public:
        SwitchPlugCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        // Backwards-compatible overload
        SwitchPlugCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : SwitchPlugCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        using BinaryCommandCapability::isOn;
        using BinaryCommandCapability::toggle;
        using BinaryCommandCapability::turnOff;
        using BinaryCommandCapability::turnOn;
    };

} // namespace iotsmartsys::core
