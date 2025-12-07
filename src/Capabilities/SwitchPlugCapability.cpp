#include "SwitchPlugCapability.h"
#include "Utils/Logger.h"

SwitchPlugCapability::SwitchPlugCapability(String capability_name, int switchPin, DigitalLogic stateLogic)
    : Capability(capability_name, SWITCH_PLUG_TYPE, SWITCH_STATE_OFF)
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

SwitchPlugCapability::SwitchPlugCapability(String capability_name, String description, String owner,
                                           String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
    switchPin = -1;
}

void SwitchPlugCapability::setup()
{
    pinMode(switchPin, OUTPUT);
}

void SwitchPlugCapability::handle()
{
    bool currentState = digitalRead(this->switchPin) == stateOn;
    if (currentState != this->switchState)
    {
        this->switchState = currentState;
        updateState(currentState ? SWITCH_STATE_ON : SWITCH_STATE_OFF);
    }
}

void SwitchPlugCapability::toggle()
{
    if (this->switchState)
        turnOff();
    else
        turnOn();
}

void SwitchPlugCapability::turnOn()
{
    digitalWrite(this->switchPin, stateOn);
}

void SwitchPlugCapability::turnOff()
{
    digitalWrite(this->switchPin, stateOff);
}

bool SwitchPlugCapability::isOn()
{
    return this->switchState;
}

void SwitchPlugCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND || state == POWER_OFF_COMMAND)
        power(state);
    else if (state == TOGGLE_COMMAND)
        toggle();
}

void SwitchPlugCapability::power(String state)
{
    state.toLowerCase();
    if (state == SWITCH_STATE_ON)
        turnOn();
    else if (state == SWITCH_STATE_OFF)
        turnOff();
}
