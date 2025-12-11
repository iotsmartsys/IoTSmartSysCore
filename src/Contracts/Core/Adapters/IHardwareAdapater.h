#pragma once

#include <string>

namespace iotsmartsys::core
{
    struct IHardwareCommand
    {
    public:
        std::string command;
    };

    struct IHardwareAdapater
    {
    public:
        virtual ~IHardwareAdapater() = default;
        virtual void setup() = 0;
        virtual bool applyCommand(const IHardwareCommand &command) = 0;
        virtual bool applyCommand(const std::string &value) = 0;
        virtual std::string getState() = 0;
    };
} // namespace iotsmartsys::core
