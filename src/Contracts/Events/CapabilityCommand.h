#pragma once

#include <string>

namespace iotsmartsys::core
{
    class DeviceCommand
    {
    public:
        std::string capability_name;
        std::string value;
    };

}