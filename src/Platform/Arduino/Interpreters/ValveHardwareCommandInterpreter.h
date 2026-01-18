#pragma once

#include "Contracts/Interpreters/IHardwareCommandInterpreter.h"

namespace iotsmartsys::core
{
    class ValveHardwareCommandInterpreter : public IHardwareCommandInterpreter
    {
    public:
        IHardwareCommand interpretCommand(const CapabilityCommand &command) override;
        std::string interpretState(IHardwareState state) override;
    };
} // namespace iotsmartsys::core