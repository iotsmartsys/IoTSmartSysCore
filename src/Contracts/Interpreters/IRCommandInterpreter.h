#pragma once

#include "Contracts/Sensors/IIRCommandSensor.h"

namespace iotsmartsys::core
{
    enum class IRControlModel
    {
        Philco,
        Springer,
        IOTSM1541
    };

    class IRCommandInterpreter
    {
    public:
        IRCommandInterpreter() = default;
        virtual ~IRCommandInterpreter() = default;

        virtual std::string interpret(const IRCommand &command) = 0;
    };

} // namespace iotsmartsys::core
