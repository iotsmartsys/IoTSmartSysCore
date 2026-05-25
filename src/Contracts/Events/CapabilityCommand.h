#pragma once

#include "Contracts/Capabilities/ICapabilityType.h"
#include <cstring>
#include <utility>
#include <vector>

namespace iotsmartsys::core
{
    class CapabilityCommand
    {
    public:
        const char *capability_name = "";
        const char *value = "";
        std::vector<std::pair<const char *, const char *>> args;

        const char *getArgValue(const char *key) const
        {
            if (!key)
                return nullptr;

            for (const auto &arg : args)
            {
                if (arg.first && std::strcmp(arg.first, key) == 0)
                    return arg.second ? arg.second : "";
            }

            return nullptr;
        }

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

        bool isCommand(const char *cmd) const
        {
            return strcmp(value, cmd) == 0;
        }
    };

}
