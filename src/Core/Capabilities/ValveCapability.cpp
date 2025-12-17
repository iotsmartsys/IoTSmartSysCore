#include "Contracts/Capabilities/ValveCapability.h"

namespace iotsmartsys::core
{
    ValveCapability::ValveCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter)
        : ICommandCapability(&hardwareAdapter, capability_name, VaLVE_ACTUATOR_TYPE, VALVE_STATE_CLOSED)
    {
    }

    void ValveCapability::setup()
    {
        ICommandCapability::setup();
    }

    void ValveCapability::handle()
    {
        std::string hwState = command_hardware_adapater ? command_hardware_adapater->getState() : std::string(VALVE_STATE_CLOSED);
        if (hwState != value)
        {
            updateState(hwState);
        }
    }

    void ValveCapability::turnOpen()
    {
        applyCommand(CapabilityCommand{type, std::string(VALVE_STATE_OPEN)});
    }

    void ValveCapability::turnClosed()
    {
        applyCommand(CapabilityCommand{type, std::string(VALVE_STATE_CLOSED)});
    }

    bool ValveCapability::isOpen() const
    {
        return value == VALVE_STATE_OPEN;
    }

    void ValveCapability::power(const std::string &state)
    {
        applyCommand(CapabilityCommand{type, state});
    }

} // namespace iotsmartsys::core
