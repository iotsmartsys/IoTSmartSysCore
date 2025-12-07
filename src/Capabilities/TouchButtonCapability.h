#pragma once
#include <Arduino.h>

#include "Capability.h"

class TouchButtonCapability : public Capability
{
public:
    TouchButtonCapability(String capability_name, int pin);

    TouchButtonCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void handle() override;
    void setup() override;
    bool isPressedButton();

private:
    int buttonPin;
    int lastState = LOW;
    int currentState;
    bool pressed = false;

    void handleTouchButton();
};
