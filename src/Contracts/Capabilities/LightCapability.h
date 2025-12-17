
#pragma once

#include "ICommandCapability.h"

namespace iotsmartsys::core
{
    class LightCapability : public ICommandCapability
    {
    public:
        LightCapability(std::string name,
                        ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);

        void toggle();
        void turnOn();
        void turnOff();
        bool isOn() const;

    private:
        void power(const std::string &state);
    };
}