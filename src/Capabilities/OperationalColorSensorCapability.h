#pragma once

#include "Capability.h"

#ifdef OPERATIONAL_COLOR_SENSOR_ENABLED
class OperationalColorSensorCapability : public Capability
{
public:
    OperationalColorSensorCapability(unsigned int readIntervalMs = 1);

    OperationalColorSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;
    String readState();

private:
    String lastState = OPERATIONAL_COLOR_SENSOR_NORMAL;
    String currentState = OPERATIONAL_COLOR_SENSOR_NORMAL;
    unsigned long lastReadTime = 0;
    unsigned long readIntervalMs = 0;
};
#endif