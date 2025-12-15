#include "Contracts/Capabilities/ValveCapability.h"

namespace iotsmartsys::core
{
    ValveCapability::ValveCapability(std::string capability_name, IHardwareAdapter &hardwareAdapter)
        : ICapability(&hardwareAdapter, capability_name, VaLVE_ACTUATOR_TYPE, VALVE_STATE_CLOSED)
    {
    }

    void ValveCapability::setup()
    {
        ICapability::setup();
    }

    void ValveCapability::handle()
    {
        std::string hwState = hardware_adapator ? hardware_adapator->getState() : std::string(VALVE_STATE_CLOSED);
        if (hwState != value)
        {
            updateState(hwState);
        }
    }

    void ValveCapability::turnOpen()
    {
        applyCommand(ICommand{type, std::string(VALVE_STATE_OPEN)});
    }

    void ValveCapability::turnClosed()
    {
        applyCommand(ICommand{type, std::string(VALVE_STATE_CLOSED)});
    }

    bool ValveCapability::isOpen() const
    {
        return value == VALVE_STATE_OPEN;
    }

    void ValveCapability::power(const std::string &state)
    {
        applyCommand(ICommand{type, state});
    }

} // namespace iotsmartsys::core
