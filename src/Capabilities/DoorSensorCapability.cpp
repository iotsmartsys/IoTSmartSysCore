#include "DoorSensorCapability.h"
#include <Arduino.h>

DoorSensorCapability::DoorSensorCapability(int doorPin)
    : Capability(DOOR_SENSOR_TYPE, DOOR_SENSOR_CLOSED), doorPin(doorPin), lastDoorState(LOW), doorState(false)
{
}

DoorSensorCapability::DoorSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), doorPin(-1), lastDoorState(LOW), doorState(false)
{
}

void DoorSensorCapability::setup()
{
    if (doorPin == -1)
        return;

    pinMode(doorPin, INPUT_PULLUP);
}

void DoorSensorCapability::handle()
{
    int actualState = digitalRead(this->doorPin);
    if (actualState != lastDoorState)
    {
        lastDoorState = actualState;
        this->doorState = actualState == HIGH;
        updateState(actualState ? DOOR_SENSOR_OPEN : DOOR_SENSOR_CLOSED);
    }
}

bool DoorSensorCapability::isOpen()
{
    return doorState;
}
