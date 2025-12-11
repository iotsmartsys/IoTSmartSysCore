#pragma once

#include <string>

namespace iotsmartsys::core
{
    struct IHardwareAdapater
    {
    public:
        virtual ~IHardwareAdapater() = default;
        virtual void setup() = 0;
        virtual bool applyCommand(const std::string &command, const std::string &value) = 0;
        virtual bool applyCommand(const std::string &value) = 0;
        virtual std::string getState() = 0;
    };
} // namespace iotsmartsys::core