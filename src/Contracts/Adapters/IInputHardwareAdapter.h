#pragma once

#include <stdint.h>
#include "IHardwareAdapter.h"

namespace iotsmartsys::core
{
    struct IInputHardwareAdapter : public IHardwareAdapter
    {
    public:
        virtual ~IInputHardwareAdapter() = default;
        virtual void setup() override = 0;
        virtual int32_t readInput() = 0;
        virtual bool digitalActive() = 0;
    };
} // namespace iotsmartsys::core
