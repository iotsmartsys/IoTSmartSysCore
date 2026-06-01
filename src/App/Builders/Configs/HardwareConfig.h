#pragma once

#include <cstdio>
#include <stdint.h>
#include "Core/Models/DigitalLogic.h"

namespace iotsmartsys::app
{
    class HardwareConfig
    {
    public:
        uint8_t GPIO;
        bool highIsOn = true;
        const char *capability_name = nullptr;
        char capabilityNameBuffer[32] = {};

        const char *makeCapabilityName(const char *baseName)
        {
            if (capability_name)
            {
                return capability_name;
            }
            else
            {
                snprintf(capabilityNameBuffer, sizeof(capabilityNameBuffer), "%s_%d", baseName, GPIO);
                capability_name = capabilityNameBuffer;
                return capability_name;
            }
        }

        explicit HardwareConfig(uint8_t gpio) : GPIO(gpio) {}
        explicit HardwareConfig(uint8_t gpio, bool highOn) : GPIO(gpio), highIsOn(highOn) {}
        explicit HardwareConfig(uint8_t gpio, bool highOn, const char *name) : GPIO(gpio), highIsOn(highOn), capability_name(name) {}
        HardwareConfig() = default;
    };

    class InputHardwareConfig
    {
    public:
        uint8_t GPIO;
        bool highIsOn = true;
        const char *capability_name = nullptr;
        long debounceTimeMs = 50;

        explicit InputHardwareConfig(uint8_t gpio) : GPIO(gpio) {}
        explicit InputHardwareConfig(uint8_t gpio, bool highOn) : GPIO(gpio), highIsOn(highOn) {}
        explicit InputHardwareConfig(uint8_t gpio, bool highOn, const char *name) : GPIO(gpio), highIsOn(highOn), capability_name(name) {}
        InputHardwareConfig() = default;
    };
} // namespace iotsmartsys::app
