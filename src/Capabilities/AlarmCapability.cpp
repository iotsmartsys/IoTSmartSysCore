#include "AlarmCapability.h"
#include <Arduino.h>
#include "Utils/Logger.h"

AlarmCapability::AlarmCapability(int alarmPin, DigitalLogic stateLogic)
    : Capability(ALARM_ACTUATOR_TYPE, ALARM_OFF), alarmPin(alarmPin), stateOn(HIGH), stateOff(LOW), lastRing(0), poweredOn(false), lastState(false)
{
    if (stateLogic == DigitalLogic::INVERSE)
    {
        stateOn = LOW;
        stateOff = HIGH;
    }
    setCallback([this](String *state)
                {
                    LOG_INFO("");
                    LOG_INFO("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state);
                });
}

AlarmCapability::AlarmCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), alarmPin(-1), stateOn(HIGH), stateOff(LOW), lastRing(0), poweredOn(false), lastState(false)
{
}

void AlarmCapability::setup()
{
    pinMode(this->alarmPin, OUTPUT);
    digitalWrite(alarmPin, stateOff);
    powerOff();
}

void AlarmCapability::handle()
{
    LOG_DEBUG("Handling " + this->capability_name + " capability.");
    if (poweredOn)
    {
        ring();
    }
    else
    {
        LOG_DEBUG("Alarm is off, not ringing.");
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
    LOG_INFO("Ringing alarm...");
    if (millis() - lastRing > ringDuration)
    {
        LOG_INFO("Alarm ringed!");
        lastRing = millis();
        digitalWrite(this->alarmPin, !digitalRead(this->alarmPin));
    }
    else
    {
        LOG_DEBUG("Alarm already ringing!");
    }
}

void AlarmCapability::setRingDuration(int duration)
{
    this->ringDuration = duration;
}

void AlarmCapability::powerOn()
{
    poweredOn = true;
    ring();
    updateState(ALARM_ON);
}

void AlarmCapability::powerOff()
{
    digitalWrite(this->alarmPin, stateOff);
    poweredOn = false;
    updateState(ALARM_OFF);
}

void AlarmCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND)
        powerOn();
    else if (state == POWER_OFF_COMMAND)
        powerOff();
    else if (state == TOGGLE_COMMAND)
        toggle();
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
