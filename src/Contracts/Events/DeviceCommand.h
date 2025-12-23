#pragma once

#include <string>

namespace iotsmartsys::core
{
    class DeviceCommand
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
    };

    class Command
    {

    public:
        std::string command;
        std::string value;
    };

}