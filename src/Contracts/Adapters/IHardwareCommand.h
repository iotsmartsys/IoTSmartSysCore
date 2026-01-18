#pragma once

#include <string>

namespace iotsmartsys::core
{
    struct IHardwareCommand
    {
    public:
        IHardwareCommand() = default;
        explicit IHardwareCommand(const std::string &cmd) : command(cmd) {}
        explicit IHardwareCommand(const char *cmd) : command(cmd) {}
        std::string getCommand() const
        {
            return command;
        }

        bool isValid() const
        {
            return !command.empty();
        }

        bool operator==(const IHardwareCommand &other) const
        {
            return command == other.command;
        }

        bool operator!=(const IHardwareCommand &other) const
        {
            return !(*this == other);
        }

        bool isEqualTo(const char *cmd) const
        {
            return command == cmd;
        }

        bool isEqualTo(const std::string &cmd) const
        {
            return command == cmd;
        }

    private:
        std::string command;
    };
} // namespace iotsmartsys::core