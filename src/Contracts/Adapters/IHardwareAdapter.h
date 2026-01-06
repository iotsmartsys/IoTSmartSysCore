#pragma once

#include <string>
#include "IHardwareCommand.h"

namespace iotsmartsys::core
{
    struct IHardwareAdapter
    {
    public:
        virtual ~IHardwareAdapter() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;
        virtual long lastStateReadMillis() const = 0;
    };
} // namespace iotsmartsys::core
