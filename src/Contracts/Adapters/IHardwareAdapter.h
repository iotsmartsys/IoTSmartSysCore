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
    };
} // namespace iotsmartsys::core
