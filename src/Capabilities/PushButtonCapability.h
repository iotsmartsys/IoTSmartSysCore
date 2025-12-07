#pragma once
#include <Arduino.h>
#include "Capability.h"

class PushButtonCapability : public Capability
{
public:
    PushButtonCapability(String capability_name, int buttonPin, int toleranceTime);
    PushButtonCapability(String capability_name, int buttonPin);
    PushButtonCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

private:
    int buttonPin;
    long lastTimeButtonPressed;
    bool buttonPressed;
    int timeTolerance;
    unsigned long lastPressTime = 0;
    unsigned long multiClickTimeout = 400;
    int clickCount = 0;

    long getTimeSinceLastButtonPressed();
    void handleButtonPress();
};
