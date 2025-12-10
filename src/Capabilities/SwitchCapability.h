#pragma once

#include "Capability.h"

class SwitchCapability : public Capability
{
public:
    SwitchCapability(String capability_name, int switchPin, DigitalLogic stateLogic = DigitalLogic::NORMAL);

    SwitchCapability(String capability_name, String description, String owner,
                     String type, String mode, String value);

    void setup() override;
    void handle() override;
    void toggle();
    void turnOn();
    void turnOff();
    bool isOn();
    void executeCommand(String state);

private:
    int switchPin;
    bool switchState = false;
    int stateOn = HIGH;
    int stateOff = LOW;

    void power(String state);
};
