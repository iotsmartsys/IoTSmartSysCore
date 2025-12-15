#pragma once

#include "CapabilityConfig.h"

namespace iotsmartsys::app
{

    struct LightConfig : public CapabilityConfig
    {
        uint8_t pin;
        bool activeHigh = true;
        bool initialOn = false;
    };

    struct AlarmConfig : public CapabilityConfig
    {
        uint8_t pin;
        int activeState = 1; // 1 = HIGH, 0 = LOW
    };

    struct DoorSensorConfig : public CapabilityConfig
    {
        uint8_t pin;
    };

    struct PirSensorConfig : public CapabilityConfig
    {
        uint8_t pin;
        int toleranceTime = 5; // in seconds
    };
} // namespace iotsmartsys::core