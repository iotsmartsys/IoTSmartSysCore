#include "ClapSensorCapability.h"

ClapSensorCapability::ClapSensorCapability(int ClapPin, int toleranceTime)
    : Capability("clap_sensor", CLAP_SENSOR_TYPE, CLAP_NO_DETECTED), ClapPin(ClapPin), lastTimeClapDetected(0), ClapDetected(false), lastState(false), timeTolerance(toleranceTime * 1000)
{
}

ClapSensorCapability::ClapSensorCapability(String capability_name, int ClapPin, int toleranceTime)
    : Capability(capability_name, CLAP_SENSOR_TYPE, CLAP_NO_DETECTED), ClapPin(ClapPin), lastTimeClapDetected(0), ClapDetected(false), lastState(false), timeTolerance(toleranceTime * 1000)
{
}

ClapSensorCapability::ClapSensorCapability(String capability_name, int ClapPin)
    : Capability(capability_name, CLAP_SENSOR_TYPE, CLAP_NO_DETECTED), ClapPin(ClapPin), lastTimeClapDetected(0), ClapDetected(false), lastState(false), timeTolerance(1000 * 5)
{
}

ClapSensorCapability::ClapSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), ClapPin(-1), lastTimeClapDetected(0), ClapDetected(false), lastState(false), timeTolerance(1000 * 5)
{
}

void ClapSensorCapability::setup()
{
    if (this->ClapPin != -1)
        pinMode(this->ClapPin, INPUT);
}

void ClapSensorCapability::handle()
{
    handleClap();
}

bool ClapSensorCapability::isClapDetected()
{
    return ClapDetected;
}

bool ClapSensorCapability::ClapSensorIsTriggered()
{
    return digitalRead(this->ClapPin) == HIGH;
}

long ClapSensorCapability::getTimeSinceLastClapDetected()
{
    return millis() - lastTimeClapDetected;
}

void ClapSensorCapability::handleClap()
{
    if (ClapSensorIsTriggered())
    {
        ClapDetected = true;
        lastTimeClapDetected = millis();
    }
    else if (getTimeSinceLastClapDetected() > timeTolerance)
    {
        ClapDetected = false;
    }
    if (lastState != ClapDetected)
    {
        updateState(ClapDetected ? CLAP_DETECTED : CLAP_NO_DETECTED);
        lastState = ClapDetected;
    }
}
