#pragma once

#include "ICapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{
    class IInputCapability : public ICapability
    {
    public:
        IInputCapability(IInputHardwareAdapter *input_hardware_adapter,
                         std::string type,
                         std::string value)
            : ICapability(input_hardware_adapter, type, value), inputHardwareAdapter(input_hardware_adapter) {}

        virtual ~IInputCapability() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;

    protected:
        IInputHardwareAdapter *inputHardwareAdapter;
    };
} // namespace iotsmartsys::core
