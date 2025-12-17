#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class SwitchCapability : public ICommandCapability
    {
    public:
        SwitchCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;

    private:
        void power(const std::string &state);
    };

} // namespace iotsmartsys::core
