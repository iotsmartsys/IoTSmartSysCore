#pragma once
#include <Arduino.h>
#include "Contracts/Interpreters/IAirConditionerInterpreter.h"

namespace iotsmartsys::core
{
    class PhilcoAirConditionerInterpreter : public IAirConditionerInterpreter
    {
    public:
        PhilcoAirConditionerInterpreter(AirConditionerModel model);
        ~PhilcoAirConditionerInterpreter() override = default;

        std::string interpret(const IRCommand &command) override;

    private:
        AirConditionerModel model;
    };

} // namespace iotsmartsys::core
