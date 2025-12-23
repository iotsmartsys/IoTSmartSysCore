#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class SwitchPlugCapability : public ICommandCapability
    {
    public:
        SwitchPlugCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        // Backwards-compatible overload
        SwitchPlugCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : SwitchPlugCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

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
