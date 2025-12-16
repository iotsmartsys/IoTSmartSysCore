#pragma once

#include "Contracts/Sensors/IIRCommandSensor.h"

namespace iotsmartsys::core
{
    enum class AirConditionerModel
    {
        Philco,
        Springer,
    };

    class IAirConditionerInterpreter
    {
    public:
        IAirConditionerInterpreter(AirConditionerModel model);
        virtual ~IAirConditionerInterpreter() = default;

        virtual std::string interpret(const IRCommand &command) = 0;

    protected:
        AirConditionerModel model;
    };

} // namespace iotsmartsys::core