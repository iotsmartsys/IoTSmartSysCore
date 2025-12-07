#include "IRProximitySensorCapability.h"
#include <Arduino.h>

IRProximitySensorCapability::IRProximitySensorCapability(String capability_name, int irPin)
    : Capability(capability_name, PROXIMITY_SENSOR_TYPE, PROXIMITY_NO_DETECTED), irPin(irPin), proximityDetected(false), lastState(false), lastSendEvent(0)
{
}

IRProximitySensorCapability::IRProximitySensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), irPin(-1), proximityDetected(false), lastState(false), lastSendEvent(0)
{
}

void IRProximitySensorCapability::setup()
{
    if (this->irPin != -1)
        pinMode(this->irPin, INPUT);
}

void IRProximitySensorCapability::handle()
{
    handleIRSensor();
}

bool IRProximitySensorCapability::irSensorIsTriggered()
{
    return digitalRead(this->irPin) == LOW;
}

void IRProximitySensorCapability::handleIRSensor()
{
    if (irSensorIsTriggered())
    {
        proximityDetected = true;
    }
    else
    {
        proximityDetected = false;
    }

    if (lastState != proximityDetected)
    {
        updateState(proximityDetected ? PROXIMITY_DETECTED : PROXIMITY_NO_DETECTED);
        lastState = proximityDetected;
    }
}
