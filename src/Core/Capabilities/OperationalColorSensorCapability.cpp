#include "Contracts/Capabilities/OperationalColorSensorCapability.h"

using namespace iotsmartsys::core;

OperationalColorSensorCapability::OperationalColorSensorCapability(IColorSensor *sensor, unsigned long readIntervalMs)
    : ICapability(nullptr, OPERATIONAL_COLOR_SENSOR_TYPE, OPERATIONAL_COLOR_SENSOR_NORMAL), sensor(sensor), lastState(), lastCheckMillis(0), readIntervalMs(readIntervalMs)
{
}

void OperationalColorSensorCapability::setup()
{
    ICapability::setup();
    if (sensor)
        sensor->setup();
}

void OperationalColorSensorCapability::handle()
{
    if (!sensor)
        return;

    unsigned long now = timeProvider.nowMs();
    if (now - lastCheckMillis >= readIntervalMs || lastState.empty())
    {
        lastCheckMillis = now;
        sensor->handle();
        std::string st = sensor->getStateString();
        if (st != lastState)
        {
            lastState = st;
            updateState(st);
        }
    }
}

// No readState() here: use ICapability::readState() to obtain ICapabilityState
