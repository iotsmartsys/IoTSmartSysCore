#pragma once

#include <stdint.h>
#include "Core/Models/DigitalLogic.h"

namespace iotsmartsys::app
{
    class HardwareConfig
    {
    public:
        uint8_t GPIO;
        bool highIsOn = true;
        const char *capability_name;
    };

    class InputHardwareConfig
    {
    public:
        uint8_t GPIO;
        bool highIsOn = true;
        const char *capability_name;
        long debounceTimeMs = 50;
    };
} // namespace iotsmartsys::app