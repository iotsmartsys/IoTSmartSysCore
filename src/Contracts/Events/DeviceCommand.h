#pragma once

#include <string>
#include <cctype>
#include "Contracts/Commands/CommandTypes.h"

namespace iotsmartsys::core
{
    class DeviceCommand
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
        std::string command_type;

        const CommandTypes getCommandType() const
        {
            if (command_type.empty())
                return CommandTypes::CAPABILITY;

            std::string normalizedType = command_type;
            for (auto &ch : normalizedType)
            {
                ch = std::toupper(static_cast<unsigned char>(ch));
            }

            if (normalizedType == "CAPABILITY")
            {
                return CommandTypes::CAPABILITY;
            }
            else if (normalizedType == "SYSTEM")
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
