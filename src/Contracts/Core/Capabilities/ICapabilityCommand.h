#pragma once

#include <string>

namespace iotsmartsys::core
{
    class ICapabilityCommand
    {
    public:
        std::string capability_name;
        std::string value;
        std::string device_id;
    };
}