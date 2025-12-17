#pragma once

#include "ICommandCapability.h"
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{

    class AlarmCapability : public ICommandCapability
    {
    public:
        AlarmCapability(ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);

        virtual void setup() override;
        virtual void handle() override;

        bool isActivated();

        void ring();
        void setRingDuration(int duration);

        void powerOn();
        void powerOff();

    private:
        int stateOn;
        int stateOff;
        unsigned long lastRing;
        bool poweredOn;
        bool lastState;
        int ringDuration = 250;

        void toggle();
    };
}