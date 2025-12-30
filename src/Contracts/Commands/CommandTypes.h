#pragma once

namespace iotsmartsys::core
{
    enum class CommandTypes
    {
        UNKNOWN,
        CAPABILITY,
        SYSTEM,
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
                return CommandTypes::UNKNOWN;
            }
        }

        static const char *toString(const CommandTypes &type)
        {
            switch (type)
            {
            case CommandTypes::CAPABILITY:
                return "CAPABILITY";
            case CommandTypes::SYSTEM:
                return "SYSTEM";
            default:
                return "UNKNOWN";
            }
        }
    };

    struct CommandTypeStrings
    {
        static constexpr const char *CAPABILITY_STR = "CAPABILITY";
        static constexpr const char *SYSTEM_STR = "SYSTEM";
    };

} // namespace iotsmartsys::core