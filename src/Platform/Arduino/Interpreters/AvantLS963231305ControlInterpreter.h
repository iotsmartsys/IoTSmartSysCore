#pragma once

#include <cstdint>
#include <string>
#include "Contracts/Interpreters/IRCommandInterpreter.h"

namespace iotsmartsys::platform::arduino
{
    class AvantLS963231305ControlInterpreter : public iotsmartsys::core::IRCommandInterpreter
    {
    public:
        AvantLS963231305ControlInterpreter() = default;
        ~AvantLS963231305ControlInterpreter() override = default;

        std::string interpret(const iotsmartsys::core::IRCommand &command) override;
    };

    struct AvantLS963231305ControlInterpreterConstants
    {
        static constexpr const char *ON_COMMAND = "On";
        static constexpr const char *OFF_COMMAND = "Off";
        static constexpr const char *MODE_COMMAND = "Mode";
        static constexpr const char *AUTO_COMMAND = "Auto";
        static constexpr const char *TIMER_30MIN_COMMAND = "30Min";
        static constexpr const char *TIMER_2H_COMMAND = "2H";
        static constexpr const char *TIMER_5H_COMMAND = "5H";
        static constexpr const char *TIMER_10H_COMMAND = "10H";
    };
}
