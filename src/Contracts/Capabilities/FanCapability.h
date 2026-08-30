#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class FanCapability : public BinaryCommandCapability
    {
    public:
        FanCapability(const char *capability_name,
                      ICommandHardwareAdapter &hardwareAdapter,
                      ICapabilityEventSink *event_sink);
        FanCapability(const std::string &capability_name,
                      ICommandHardwareAdapter &hardwareAdapter,
                      ICapabilityEventSink *event_sink)
            : FanCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        using BinaryCommandCapability::isOn;
        using BinaryCommandCapability::power;
        using BinaryCommandCapability::toggle;
        using BinaryCommandCapability::turnOff;
        using BinaryCommandCapability::turnOn;
    };

} // namespace iotsmartsys::core
