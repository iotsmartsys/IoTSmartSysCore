#include "PirSensorCapability.h"

PirSensorCapability::PirSensorCapability(int pirPin, int toleranceTime)
    : Capability(PIR_SENSOR_TYPE, PIR_NO_DETECTED), pirPin(pirPin), lastTimePresenceDetected(0), presenceDetected(false), lastState(false), lastSendEvent(0), timeTolerance(toleranceTime * 1000 * 60)
{
}

PirSensorCapability::PirSensorCapability(String capability_name, int pirPin)
    : Capability(capability_name, PIR_SENSOR_TYPE, PIR_NO_DETECTED), pirPin(pirPin), lastTimePresenceDetected(0), presenceDetected(false), lastState(false), lastSendEvent(0), timeTolerance(1000 * 60 * 1)
{
}

PirSensorCapability::PirSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), pirPin(-1), lastTimePresenceDetected(0), presenceDetected(false), lastState(false), lastSendEvent(0), timeTolerance(1000 * 60 * 1)
{
}

void PirSensorCapability::setup()
{
    setTimeBetweenUpdates(0); 
    if (this->pirPin != -1)
        pinMode(this->pirPin, INPUT);
}

void PirSensorCapability::handle()
{
    handlePir();
}

bool PirSensorCapability::isPresenceDetected()
{
    return presenceDetected;
}

bool PirSensorCapability::pirSensorIsTriggered()
{
    return digitalRead(this->pirPin) == HIGH;
}

long PirSensorCapability::getTimeSinceLastPresenceDetected()
{
    return millis() - lastTimePresenceDetected;
}

void PirSensorCapability::handlePir()
{
    if (pirSensorIsTriggered())
    {
        #ifdef DEBUG
        LOG_PRINTLN("Last State: " + String(lastState));
        LOG_PRINTLN("PIR Sensor triggered");
        #endif
        presenceDetected = true;
        lastTimePresenceDetected = millis();
    }
    else if (getTimeSinceLastPresenceDetected() > timeTolerance)
    {
        #ifdef DEBUG
        LOG_PRINTLN("PIR Sensor no longer triggered");
        #endif
        presenceDetected = false;
    }
    if (lastState != presenceDetected)
    {
        #ifdef DEBUG
        LOG_PRINTF("PIR Sensor state changed: %s\n", presenceDetected ? "Detected" : "Not Detected");
        #endif
        updateState(presenceDetected ? PIR_DETECTED : PIR_NO_DETECTED);
        lastState = presenceDetected;
    }
}
