#include "Contracts/Capabilities/AlarmCapability.h"

namespace iotsmartsys::core
{
    AlarmCapability::AlarmCapability(long ringDurationMs, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapter, event_sink, ALARM_ACTUATOR_TYPE, ALARM_OFF),
          ringDuration(ringDurationMs),
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
        if (poweredOn && ringDuration > 0)
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
        command_hardware_adapter.applyCommand(POWER_ON_COMMAND);
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
            if (strcmp(command.value, ALARM_RING) == 0)
            {
                if (strcmp(command.args1, ALARM_RING_DURATION) == 0)
                {
                    int duration = atoi(command.args1value);
                    setRingDuration(duration);
                }
                else
                {
                    ring();
                }
            }
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
