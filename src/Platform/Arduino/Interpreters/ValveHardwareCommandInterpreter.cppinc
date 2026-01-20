#include "ValveHardwareCommandInterpreter.h"

namespace iotsmartsys::core
{
    IHardwareCommand ValveHardwareCommandInterpreter::interpretCommand(const CapabilityCommand &command)
    {
        const char *command_value = "";
        if (strcmp(command.value, VALVE_STATE_OPEN) == 0)
        {
            command_value = POWER_ON_COMMAND;
        }
        else if (strcmp(command.value, VALVE_STATE_CLOSED) == 0)
        {
            command_value = POWER_OFF_COMMAND;
        }
        else if (strcmp(command.value, TOGGLE_COMMAND) == 0)
        {
            command_value = TOGGLE_COMMAND;
        }

        return IHardwareCommand(command_value);
    }

    std::string ValveHardwareCommandInterpreter::interpretState(IHardwareState state)
    {
        if (state.value == POWER_ON_COMMAND)
        {
            return VALVE_STATE_OPEN;
        }
        else if (state.value == POWER_OFF_COMMAND)
        {
            return VALVE_STATE_CLOSED;
        }

        return "unknown";
    }
} // namespace iotsmartsys::core
