#pragma once

#include "Capability.h"

class PirSensorCapability : public Capability
{
public:
    PirSensorCapability(int pirPin, int toleranceTime);
    PirSensorCapability(String capability_name, int pirPin);
    PirSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    bool isPresenceDetected();

private:
    int pirPin;
    long lastTimePresenceDetected;
    bool presenceDetected;
    bool lastState;
    long lastSendEvent;
    int timeTolerance;

    bool pirSensorIsTriggered();
    long getTimeSinceLastPresenceDetected();
    void handlePir();
};
