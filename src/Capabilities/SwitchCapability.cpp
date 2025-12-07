#include "SwitchCapability.h"
#include "Utils/Logger.h"

SwitchCapability::SwitchCapability(String capability_name, int switchPin, DigitalLogic stateLogic)
    : Capability(capability_name, SWITCH_TYPE, SWITCH_STATE_OFF)
{
    this->switchPin = switchPin;

    if (stateLogic == DigitalLogic::INVERSE)
    {
        stateOn = LOW;
        stateOff = HIGH;
    }

    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state); });
}

SwitchCapability::SwitchCapability(String capability_name, String description, String owner,
                                   String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
    switchPin = -1;
}

void SwitchCapability::setup()
{
    pinMode(switchPin, OUTPUT);
}

void SwitchCapability::handle()
{
    bool currentState = digitalRead(this->switchPin) == stateOn;
    if (currentState != this->switchState)
    {
        this->switchState = currentState;
        updateState(currentState ? SWITCH_STATE_ON : SWITCH_STATE_OFF);
    }
}

void SwitchCapability::toggle()
{
    if (this->switchState)
        turnOff();
    else
        turnOn();
}

void SwitchCapability::turnOn()
{
    digitalWrite(this->switchPin, stateOn);
}

void SwitchCapability::turnOff()
{
    digitalWrite(this->switchPin, stateOff);
}

bool SwitchCapability::isOn()
{
    return this->switchState;
}

void SwitchCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND || state == POWER_OFF_COMMAND)
        power(state);
    else if (state == TOGGLE_COMMAND)
        toggle();
}

void SwitchCapability::power(String state)
{
    state.toLowerCase();
    if (state == SWITCH_STATE_ON)
        turnOn();
    else if (state == SWITCH_STATE_OFF)
        turnOff();
}
