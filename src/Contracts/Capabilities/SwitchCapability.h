#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class SwitchCapability : public BinaryCommandCapability
    {
    public:
        SwitchCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        SwitchCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : SwitchCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        void handle() override;
        using BinaryCommandCapability::isOn;
        using BinaryCommandCapability::toggle;
        using BinaryCommandCapability::turnOff;
        using BinaryCommandCapability::turnOn;
    };

} // namespace iotsmartsys::core
