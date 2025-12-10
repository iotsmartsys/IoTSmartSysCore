#include "RGBLightCapability.h"

#ifdef RGB_ENABLED

RGBLightCapability::RGBLightCapability(int lightPin)
    : Capability(LIGHT_ACTUATOR_TYPE, LIGHT_STATE_OFF), lightPin(lightPin), lightState(false), COLOR_R(102), COLOR_G(178), COLOR_B(178), pixels(NUMPIXELS, lightPin, NEO_GRB + NEO_KHZ800)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state); });
}

RGBLightCapability::RGBLightCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), lightPin(-1), lightState(false), COLOR_R(102), COLOR_G(178), COLOR_B(178), pixels(NUMPIXELS, lightPin, NEO_GRB + NEO_KHZ800)
{
}

void RGBLightCapability::setup()
{
    pixels.begin();
    pixels.clear();
    pixels.show();
    turnOff();
}

void RGBLightCapability::handle()
{
    
}

void RGBLightCapability::toggle()
{
    if (this->lightState)
        turnOff();
    else
        turnOn();
}

void RGBLightCapability::turnOn()
{
    pixels.show();
    this->lightState = true;
    updateState(LIGHT_STATE_ON);
}

void RGBLightCapability::turnOff()
{
    pixels.clear();
    pixels.show();
    this->lightState = false;
    updateState(LIGHT_STATE_OFF);
}

void RGBLightCapability::setColor(int r, int g, int b)
{
    pixels.setPixelColor(0, pixels.Color(r, g, b));
}

bool RGBLightCapability::isOn()
{
    return this->lightState;
}

void RGBLightCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND || state == POWER_OFF_COMMAND)
        power(state);
    else if (state == TOGGLE_COMMAND)
        toggle();
}

void RGBLightCapability::power(String state)
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

#endif
