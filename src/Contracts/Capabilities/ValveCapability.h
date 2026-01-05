#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class ValveCapability : public BinaryCommandCapability
    {
    public:
        ValveCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        ValveCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : ValveCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        void handle() override;

        void turnOpen();
        void turnClosed();
        bool isOpen() const;
        using BinaryCommandCapability::toggle;
        using BinaryCommandCapability::turnOff;
        using BinaryCommandCapability::turnOn;
        using BinaryCommandCapability::isOn;
    };

} // namespace iotsmartsys::core
