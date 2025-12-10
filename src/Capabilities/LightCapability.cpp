#include "LightCapability.h"

LightCapability::LightCapability(int lightPin, DigitalLogic digitalLogic)
    : Capability(LIGHT_ACTUATOR_TYPE, LIGHT_STATE_OFF), lightPin(lightPin), backLightPin(-1), lightState(false), digitalLogic(digitalLogic)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state);
                });
}

LightCapability::LightCapability(String capability_name, int lightPin)
    : Capability(capability_name, LIGHT_ACTUATOR_TYPE, LIGHT_STATE_OFF), lightPin(lightPin), backLightPin(-1), lightState(false), digitalLogic(DigitalLogic::NORMAL)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state);
                });
}

LightCapability::LightCapability(String capability_name, int lightPin, int backLightPin)
    : Capability(capability_name, LIGHT_ACTUATOR_TYPE, LIGHT_STATE_OFF), lightPin(lightPin), backLightPin(backLightPin), lightState(false), digitalLogic(DigitalLogic::NORMAL)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state);
                });
}

LightCapability::LightCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), lightPin(-1), backLightPin(-1), lightState(false), digitalLogic(DigitalLogic::NORMAL)
{
}

void LightCapability::setup()
{
    pinMode(lightPin, OUTPUT);
    if (this->backLightPin > -1)
        pinMode(backLightPin, OUTPUT);
    turnOff();
}

void LightCapability::handle()
{
    bool currentState = digitalRead(this->lightPin) == (digitalLogic == DigitalLogic::INVERSE ? LOW : HIGH);
    if (currentState != this->lightState)
    {
        this->lightState = currentState;
        updateState(currentState ? LIGHT_STATE_ON : LIGHT_STATE_OFF);
    }
}

void LightCapability::toggle()
{
    if (this->lightState)
        turnOff();
    else
        turnOn();
}

void LightCapability::turnOn()
{
    digitalWrite(this->lightPin, digitalLogic == DigitalLogic::INVERSE ? LOW : HIGH);

    this->lightState = true;
    updateState(LIGHT_STATE_ON);
}

void LightCapability::turnOff()
{
    digitalWrite(this->lightPin, digitalLogic == DigitalLogic::INVERSE ? HIGH : LOW);
    this->lightState = false;
    updateState(LIGHT_STATE_OFF);
}

bool LightCapability::isOn()
{
    return this->lightState;
}

void LightCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND || state == POWER_OFF_COMMAND)
        power(state);
    else if (state == TOGGLE_COMMAND)
        toggle();
}

void LightCapability::power(String state)
{
    state.toLowerCase();
    if (state == LIGHT_STATE_ON)
    {
        turnOn();
    }
    else if (state == LIGHT_STATE_OFF)
    {
        turnOff();
    }
}
