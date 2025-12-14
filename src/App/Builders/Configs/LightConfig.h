#pragma once

#include "CapabilityConfig.h"

namespace iotsmartsys::app
{

    struct LightConfig : public CapabilityConfig
    {
        uint8_t pin = 2;
        bool activeHigh = true;
        bool initialOn = false;
    };

    struct AlarmConfig : public CapabilityConfig
    {
        uint8_t pin = 3;
        int activeState = 1; // 1 = HIGH, 0 = LOW
    };
} // namespace iotsmartsys::core