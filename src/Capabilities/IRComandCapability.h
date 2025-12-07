#pragma once

#include <Arduino.h>
#include "Capability.h"

class IRCommandCapability : public Capability
{
public:
    IRCommandCapability(String capability_name, int irPin);
    IRCommandCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

private:
    int irPin;
    uint64_t currentState;
    uint64_t lastState;

    void setACMode(String mode, uint8_t temp = 0);
};
