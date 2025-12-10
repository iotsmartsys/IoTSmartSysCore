#pragma once
#include <Arduino.h>
#include "Capability.h"

class ValveCapability : public Capability
{
public:
    ValveCapability(String capability_name, int pin, DigitalLogic stateLogic = DigitalLogic::NORMAL);

    ValveCapability(String capability_name, String description, String owner,
                    String type, String mode, String value);

    void setup() override;
    void handle() override;
    void turnOpen();
    void turnClosed();
    bool isOpen();

private:
    int pin;
    bool lastState = false;
    int stateOpen = HIGH;
    int stateClosed = LOW;

    void power(String state);
};
