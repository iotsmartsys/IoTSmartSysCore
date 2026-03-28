#pragma once
#include <Arduino.h>
#include "Contracts/Interpreters/IRCommandInterpreter.h"

namespace iotsmartsys::platform::arduino
{

    class IoTSM1541ControlInterpreter : public iotsmartsys::core::IRCommandInterpreter
    {

    public:
        IoTSM1541ControlInterpreter();
        ~IoTSM1541ControlInterpreter() override = default;

        std::string interpret(const iotsmartsys::core::IRCommand &command) override;
    };

    struct IoTSM1541ControlInterpreterConstants
    {
        static constexpr const char *POWER_COMMAND = "Power";
        static constexpr const char *COMMAND_1 = "Command 1";
        static constexpr const char *COMMAND_2 = "Command 2";
        static constexpr const char *COMMAND_3 = "Command 3";
        static constexpr const char *COMMAND_4 = "Command 4";
        static constexpr const char *LIGHT_COMMAND = "Light";
    };

} // namespace iotsmartsys::platform::arduino
