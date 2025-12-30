#pragma once

#include <string>
#include "Contracts/Commands/CommandTypes.h"

namespace iotsmartsys::core
{
    class DeviceCommand
    {
    public:
        const char *capability_name;
        const char *device_id;
        const char *value;
        const char *command_type;

        const CommandTypes getCommandType() const
        {
            return CommandTypes::CAPABILITY;
            if (command_type == nullptr)
                return CommandTypes::CAPABILITY;

            if (std::string(command_type) == "CAPABILITY")
            {
                return CommandTypes::CAPABILITY;
            }
            else if (std::string(command_type) == "SYSTEM")
            {
                return CommandTypes::SYSTEM;
            }
            else
            {
                return CommandTypes::CAPABILITY;
            }
        }
    };

}