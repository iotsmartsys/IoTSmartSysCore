#include "ValveCapability.h"
#include "Utils/Logger.h"

ValveCapability::ValveCapability(String capability_name, int pin, DigitalLogic stateLogic)
    : Capability(capability_name, VaLVE_ACTUATOR_TYPE, stateLogic == DigitalLogic::NORMAL ? VALVE_STATE_CLOSED : VALVE_STATE_OPEN)
{
    if (stateLogic == DigitalLogic::INVERSE)
    {
        stateOpen = LOW;
        stateClosed = HIGH;
    }
    this->pin = pin;
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    power(*state); });
}

ValveCapability::ValveCapability(String capability_name, String description, String owner,
                                 String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
    pin = -1;
}

void ValveCapability::setup()
{
    pinMode(pin, OUTPUT);
    updateState(digitalRead(pin) == stateOpen ? VALVE_STATE_OPEN : VALVE_STATE_CLOSED);
}

void ValveCapability::handle()
{
    bool currentState = digitalRead(this->pin) == stateOpen;
    if (currentState != this->lastState)
    {
        this->lastState = currentState;
        updateState(currentState ? VALVE_STATE_OPEN : VALVE_STATE_CLOSED);
    }
}

void ValveCapability::turnOpen()
{
    digitalWrite(this->pin, stateOpen);
    updateState(VALVE_STATE_OPEN);
}

void ValveCapability::turnClosed()
{
    digitalWrite(this->pin, stateClosed);
    updateState(VALVE_STATE_CLOSED);
}

bool ValveCapability::isOpen()
{
    return this->lastState;
}

void ValveCapability::power(String state)
{
    state.toLowerCase();
    if (state == VALVE_STATE_OPEN)
        turnOpen();
    else if (state == VALVE_STATE_CLOSED)
        turnClosed();
}
