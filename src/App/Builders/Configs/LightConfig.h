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
} // namespace iotsmartsys::core