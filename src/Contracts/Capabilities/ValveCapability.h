#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class ValveCapability : public ICommandCapability
    {
    public:
        ValveCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);
        ValveCapability(const std::string &capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
            : ValveCapability(capability_name.c_str(), hardwareAdapter, event_sink) {}

        void handle() override;

        void turnOpen();
        void turnClosed();
        bool isOpen() const;

    private:
        void power(const char *state);
        void power(const std::string &state) { power(state.c_str()); }
    };

} // namespace iotsmartsys::core
