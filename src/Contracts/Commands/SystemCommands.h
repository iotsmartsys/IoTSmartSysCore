#pragma once

#include <string>

namespace iotsmartsys::core
{
    enum class SystemCommands
    {
        UNKNOWN,
        REBOOT,
        FACTORY_RESET,
        UPDATE_FIRMWARE
    };

    struct SystemCommandStrings
    {
        static constexpr const char *UNKNOWN = "UNKNOWN";
        static constexpr const char *REBOOT = "REBOOT";
        static constexpr const char *FACTORY_RESET = "FACTORY_RESET";
        static constexpr const char *UPDATE_FIRMWARE = "UPDATE_FIRMWARE";
    };

    struct SystemCommandUtils
    {
        static const char *toString(const SystemCommands &command)
        {
            switch (command)
            {
            case SystemCommands::REBOOT:
                return SystemCommandStrings::REBOOT;
            case SystemCommands::FACTORY_RESET:
                return SystemCommandStrings::FACTORY_RESET;
            case SystemCommands::UPDATE_FIRMWARE:
                return SystemCommandStrings::UPDATE_FIRMWARE;
            default:
                return SystemCommandStrings::UNKNOWN;
            }
        }

        static SystemCommands fromString(const char *commandStr)
        {
            if (!commandStr)
            {
                return SystemCommands::UNKNOWN;
            }

            std::string normalizedCommand = commandStr;
            for (auto &ch : normalizedCommand)
            {
                ch = std::toupper(static_cast<unsigned char>(ch));
            }

            if (normalizedCommand == SystemCommandStrings::REBOOT)
            {
                return SystemCommands::REBOOT;
            }
            else if (normalizedCommand == SystemCommandStrings::FACTORY_RESET)
            {
                return SystemCommands::FACTORY_RESET;
            }
            else if (normalizedCommand == SystemCommandStrings::UPDATE_FIRMWARE)
            {
                return SystemCommands::UPDATE_FIRMWARE;
            }
            else
            {
                return SystemCommands::UNKNOWN;
            }
        }
    };

}