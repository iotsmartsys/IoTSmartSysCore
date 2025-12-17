#include "Contracts/Capabilities/AlarmCapability.h"

namespace iotsmartsys::core
{
    AlarmCapability::AlarmCapability(ICommandHardwareAdapter &hardwareAdapter)
        : ICommandCapability(&hardwareAdapter, ALARM_ACTUATOR_TYPE, ALARM_OFF),
          alarmPin(-1),
          stateOn(1),
          stateOff(0),
          lastRing(0),
          poweredOn(false),
          lastState(false)
    {
    }

    void AlarmCapability::setup()
    {
        // Prepare the underlying hardware and reset state to off
        ICapability::setup();
        poweredOn = false;
        lastState = false;
        lastRing = timeProvider.nowMs();
        updateState(ALARM_OFF);
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
        logger.info("AlarmCapability", "Ringing alarm...");
        if (timeProvider.nowMs() - lastRing > ringDuration)
        {
            logger.info("AlarmCapability", "Alarm ringed!");
            lastRing = timeProvider.nowMs();
            toggle();
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
        if (command_hardware_adapater)
        {
            command_hardware_adapater->applyCommand(POWER_ON_COMMAND);
        }
        poweredOn = true;
        ring();
        updateState(ALARM_ON);
    }

    void AlarmCapability::powerOff()
    {
        if (command_hardware_adapater)
        {
            command_hardware_adapater->applyCommand(POWER_OFF_COMMAND);
        }
        poweredOn = false;
        updateState(ALARM_OFF);
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
