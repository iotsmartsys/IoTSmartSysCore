#pragma once

#include "ICapability.h"
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{

    class AlarmCapability : public ICapability
    {
    public:
        AlarmCapability(IHardwareAdapter &hardwareAdapter);

        void setup() override;
        void handle() override;

        bool isActivated();

        void ring();
        void setRingDuration(int duration);

        void powerOn();
        void powerOff();

    private:
        int alarmPin;
        int stateOn;
        int stateOff;
        unsigned long lastRing;
        bool poweredOn;
        bool lastState;
        int ringDuration = 250;

        void toggle();
    };
}