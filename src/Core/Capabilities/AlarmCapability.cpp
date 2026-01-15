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
        else
        {
            logger.debug("AlarmCapability", "Alarm is off, not ringing.");
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
        logger.debug("AlarmCapability", "Ringing alarm...");
        if (timeProvider.nowMs() - lastRing > ringDuration)
        {
            logger.debug("AlarmCapability", "Alarm ringed!");
            lastRing = timeProvider.nowMs();
            command_hardware_adapter.applyCommand(TOGGLE_COMMAND);
        }
        else
        {
            logger.debug("AlarmCapability", "Alarm already ringing!");
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
        logger.info("AlarmCapability", "Applying command: %s", command.value);
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
            logger.warn("AlarmCapability", "Unknown command value: %s", command.value);
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
