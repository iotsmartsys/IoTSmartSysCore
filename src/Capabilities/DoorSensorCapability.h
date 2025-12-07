#pragma once

#include "Capability.h"

class DoorSensorCapability : public Capability
{
public:
    DoorSensorCapability(int doorPin);
    DoorSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    bool isOpen();

private:
    int doorPin;
    int lastDoorState;
    bool doorState;
};
