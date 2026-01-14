#pragma once

#include "Contracts/Capabilities/ICapabilityType.h"
#include <cstring>

namespace iotsmartsys::core
{
    class CapabilityCommand
    {
    public:
        const char *capability_name;
        const char *value;

        bool isToggle() const
        {
            return strcmp(value, TOGGLE_COMMAND) == 0;
        }

        bool isPowerOn() const
        {
            return strcmp(value, POWER_ON_COMMAND) == 0;
        }

        bool isPowerOff() const
        {
            return strcmp(value, POWER_OFF_COMMAND) == 0;
        }
    };

}