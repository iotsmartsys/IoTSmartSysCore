#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class SwitchCapability : public ICommandCapability
    {
    public:
        SwitchCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        SwitchCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : SwitchCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        void setup() override;
        void handle() override;

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;

    private:
        void power(const char *state);
        void power(const std::string &state) { power(state.c_str()); }
    };

} // namespace iotsmartsys::core
