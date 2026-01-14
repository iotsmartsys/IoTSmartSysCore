#pragma once

#include <stdint.h>
#include "IHardwareAdapter.h"

namespace iotsmartsys::core
{
    enum class InputPullMode
    {
        NONE = 0,
        PULL_UP = 1,
        PULL_DOWN = 2
    };

    enum struct HardwareDigitalLogic
    {
        HIGH_IS_ON,
        LOW_IS_ON
    };

    struct IInputHardwareAdapter : public IHardwareAdapter
    {
    public:
        virtual ~IInputHardwareAdapter() = default;
        virtual void setup() override = 0;
        virtual int32_t readInput() = 0;
        virtual bool digitalActive() = 0;
    };
} // namespace iotsmartsys::core
