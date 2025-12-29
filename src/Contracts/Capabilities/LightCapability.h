
#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class LightCapability : public ICommandCapability
    {
    public:
        LightCapability(const char *name,
                        ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        LightCapability(const std::string &name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : LightCapability(name.c_str(), hardwareAdapter, event_sink) {}

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;

    private:
        void power(const char *state);
        void power(const std::string &state) { power(state.c_str()); }
    };
}