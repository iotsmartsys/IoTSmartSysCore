#pragma once

#include <string>

namespace iotsmartsys::core
{
    class ICapabilityCommand
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
    };

    class ICommand
    {

    public:
        std::string command;
        std::string value;
    };

}