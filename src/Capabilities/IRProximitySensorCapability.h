#pragma once

#include <Arduino.h>
#include "Capability.h"

class IRProximitySensorCapability : public Capability
{
public:
    IRProximitySensorCapability(String capability_name, int irPin);
    IRProximitySensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    bool irSensorIsTriggered();

private:
    int irPin;
    bool proximityDetected;
    bool lastState;
    long lastSendEvent;

    void handleIRSensor();
};
