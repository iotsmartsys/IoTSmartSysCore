#pragma once

#include <string>
#include <cctype>
#include "Contracts/Commands/CommandTypes.h"
#include "Contracts/Commands/SystemCommands.h"

namespace iotsmartsys::core
{
    class DeviceCommand
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
        std::string type;

        const CommandTypes getCommandType() const
        {
            if (type.empty())
                return CommandTypes::UNKNOWN;

            return CommandTypeUtils::fromString(type.c_str());
        }

        const SystemCommands getSystemCommand() const
        {
            if (value.empty())
                return SystemCommands::UNKNOWN;

            return SystemCommandUtils::fromString(value.c_str());
        }
    };

}
