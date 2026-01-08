
#pragma once

#include "Core/Capabilities/CapabilityHelpers.h"

namespace iotsmartsys::core
{
    class LightCapability : public BinaryCommandCapability
    {
    public:
        LightCapability(const char *name,
                        ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        LightCapability(const std::string &name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : LightCapability(name.c_str(), hardwareAdapter, event_sink) {}

        void toggle();
        void turnOn();
    };
}
