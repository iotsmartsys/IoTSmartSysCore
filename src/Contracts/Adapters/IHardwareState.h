#pragma once

#include <string>

namespace iotsmartsys::core
{
    struct IHardwareState
    {
    public:
        virtual ~IHardwareState() = default;
        std::string value;
    };
} // namespace iotsmartsys::core