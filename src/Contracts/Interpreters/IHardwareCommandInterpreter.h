#pragma once

#include "Contracts/Events/CapabilityCommand.h"
#include "Contracts/Adapters/IHardwareCommand.h"
#include "Contracts/Adapters/IHardwareState.h"

namespace iotsmartsys::core
{
    class IHardwareCommandInterpreter
    {
    public:
        virtual ~IHardwareCommandInterpreter() = default;

        virtual IHardwareCommand interpretCommand(const CapabilityCommand &command) = 0;
        virtual std::string interpretState(IHardwareState state) = 0;
    };
} // namespace iotsmartsys::core
