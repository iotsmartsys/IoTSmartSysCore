#pragma once

namespace iotsmartsys::core
{
    enum class CommandTypes
    {
        UNKNOWN,
        CAPABILITY,
        SYSTEM,
    };

    struct CommandTypeStrings
    {
        static constexpr const char *UNKNOWN_STR = "UNKNOWN";
        static constexpr const char *CAPABILITY_STR = "CAPABILITY";
        static constexpr const char *SYSTEM_STR = "SYSTEM";
    };

    struct CommandTypeUtils
    {
        static CommandTypes fromString(const char *typeStr)
        {
            if (!typeStr)
            {
                return CommandTypes::UNKNOWN;
            }

            std::string normalizedType = typeStr;
            for (auto &ch : normalizedType)
            {
                ch = std::toupper(static_cast<unsigned char>(ch));
            }

            if (normalizedType == CommandTypeStrings::CAPABILITY_STR)
            {
                return CommandTypes::CAPABILITY;
            }
            else if (normalizedType == CommandTypeStrings::SYSTEM_STR)
            {
                return CommandTypes::SYSTEM;
            }
            else
            {
                return CommandTypes::UNKNOWN;
            }
        }

        static const char *toString(const CommandTypes &type)
        {
            switch (type)
            {
            case CommandTypes::CAPABILITY:
                return CommandTypeStrings::CAPABILITY_STR;
            case CommandTypes::SYSTEM:
                return CommandTypeStrings::SYSTEM_STR;
            default:
                return CommandTypeStrings::UNKNOWN_STR;
            }
        }
    };

} // namespace iotsmartsys::core