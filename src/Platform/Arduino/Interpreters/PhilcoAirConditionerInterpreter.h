#pragma once
#include <Arduino.h>
#include "Contracts/Interpreters/IRCommandInterpreter.h"

namespace iotsmartsys::core
{
    class PhilcoAirConditionerInterpreter : public IRCommandInterpreter
    {
    public:
        PhilcoAirConditionerInterpreter();
        ~PhilcoAirConditionerInterpreter() override = default;

        std::string interpret(const IRCommand &command) override;

    };

} // namespace iotsmartsys::core
