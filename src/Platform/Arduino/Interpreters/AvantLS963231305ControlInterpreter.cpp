#include "AvantLS963231305ControlInterpreter.h"

namespace iotsmartsys::platform::arduino
{
    std::string AvantLS963231305ControlInterpreter::interpret(const iotsmartsys::core::IRCommand &command)
    {
        switch (command.code)
        {
        case 0xFF22DD:
            return AvantLS963231305ControlInterpreterConstants::ON_COMMAND;
        case 0xFFC23D:
            return AvantLS963231305ControlInterpreterConstants::OFF_COMMAND;
        case 0xFFE01F:
            return AvantLS963231305ControlInterpreterConstants::MODE_COMMAND;
        case 0xFF906F:
            return AvantLS963231305ControlInterpreterConstants::AUTO_COMMAND;
        case 0xFF6897:
            return AvantLS963231305ControlInterpreterConstants::TIMER_30MIN_COMMAND;
        case 0xFFB04F:
            return AvantLS963231305ControlInterpreterConstants::TIMER_2H_COMMAND;
        case 0xFF10EF:
            return AvantLS963231305ControlInterpreterConstants::TIMER_5H_COMMAND;
        case 0xFF5AA5:
            return AvantLS963231305ControlInterpreterConstants::TIMER_10H_COMMAND;
        default:
            return "unknown";
        }
    }
}
