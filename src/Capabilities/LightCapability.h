#pragma once

#include "Capability.h"

class LightCapability : public Capability
{
public:
    LightCapability(int lightPin, DigitalLogic digitalLogic = DigitalLogic::NORMAL);
    LightCapability(String capability_name, int lightPin);
    LightCapability(String capability_name, int lightPin, int backLightPin);
    LightCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    void toggle();
    void turnOn();
    void turnOff();
    bool isOn();
    void executeCommand(String state);

private:
    int lightPin;
    int backLightPin;
    bool lightState;
    DigitalLogic digitalLogic;

    void power(String state);
};
