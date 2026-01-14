#pragma once

#include "ICommandCapability.h"
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{

    class AlarmCapability : public ICommandCapability
    {
    public:
        AlarmCapability(ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);

        virtual void handle() override;

        bool isActivated();

        void ring();
        void setRingDuration(int duration);

        void powerOn();
        void powerOff();
        void applyCommand(CapabilityCommand command) override;

    private:
        int stateOn;
        int stateOff;
        unsigned long lastRing;
        bool poweredOn;
        bool lastState;
        int ringDuration = 500;

        void toggle();
    };
}