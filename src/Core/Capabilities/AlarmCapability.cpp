#include "Contracts/Capabilities/AlarmCapability.h"

namespace iotsmartsys::core
{
    AlarmCapability::AlarmCapability(ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapter, event_sink, ALARM_ACTUATOR_TYPE, ALARM_OFF),
          stateOn(1),
          stateOff(0),
          lastRing(0),
          poweredOn(false),
          lastState(false)
    {
        lastRing = timeProvider.nowMs();
    }

    void AlarmCapability::handle()
    {
        if (poweredOn)
        {
            ring();
        }

        if (lastState != poweredOn)
        {
            updateState(poweredOn ? ALARM_ON : ALARM_OFF);
            lastState = poweredOn;
        }
    }

    bool AlarmCapability::isActivated()
    {
        return poweredOn;
    }

    void AlarmCapability::ring()
    {
        if (timeProvider.nowMs() - lastRing > ringDuration)
        {
            lastRing = timeProvider.nowMs();
            command_hardware_adapter.applyCommand(TOGGLE_COMMAND);
        }
    }

    void AlarmCapability::setRingDuration(int duration)
    {
        this->ringDuration = duration;
    }

    void AlarmCapability::powerOn()
    {
        poweredOn = true;
    }

    void AlarmCapability::applyCommand(CapabilityCommand command)
    {
        if (command.isPowerOn())
        {
            powerOn();
        }
        else if (command.isPowerOff())
        {
            powerOff();
        }
        else if (command.isToggle())
        {
            toggle();
        }
        else
        {
        }
    }

    void AlarmCapability::powerOff()
    {
        poweredOn = false;
        command_hardware_adapter.applyCommand(POWER_OFF_COMMAND);
    }

    void AlarmCapability::toggle()
    {
        if (poweredOn)
        
        {
            powerOff();
        }
        else
        {
            powerOn();
        }
    }

}
