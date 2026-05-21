#pragma once

#include "ICommandCapability.h"
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    #define ALARM_RING "ring"
    #define ALARM_RING_DURATION "duration"

    class AlarmCapability : public ICommandCapability
    {
    public:
        AlarmCapability(std::string capability_name, long ringDurationMs, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink);

        virtual void handle() override;

        bool isActivated();
        bool isOn() const { return poweredOn; }
        bool isOff() const { return !poweredOn; }

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

    public:
        long ringDuration = 500;

        void toggle();
    };
}