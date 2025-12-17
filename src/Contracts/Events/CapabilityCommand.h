#pragma once

#include <string>

namespace iotsmartsys::core
{
    class CapabilityCommand
    {
    public:
        std::string capability_name;
        std::string device_id;
        std::string value;
    };

}