#include "IoTSM1541ControlInterpreter.h"

namespace iotsmartsys::platform::arduino
{
    IoTSM1541ControlInterpreter::IoTSM1541ControlInterpreter()
    {
    }

    std::string IoTSM1541ControlInterpreter::interpret(const iotsmartsys::core::IRCommand &command)
    {
        switch (command.code)
        {
        case 0xFFA25D:
            return IoTSM1541ControlInterpreterConstants::POWER_COMMAND;
        case 0xFF629D:
            return IoTSM1541ControlInterpreterConstants::COMMAND_1;
        case 0xFFE21D:
            return IoTSM1541ControlInterpreterConstants::COMMAND_2;
        case 0xFFE01F:
            return IoTSM1541ControlInterpreterConstants::COMMAND_3;
        case 0xFFA857:
            return IoTSM1541ControlInterpreterConstants::COMMAND_4;
        case 0xFF906F:
            return IoTSM1541ControlInterpreterConstants::LIGHT_COMMAND;
        default:
            return "unknown";
        }
    }

} // namespace iotsmartsys::core
