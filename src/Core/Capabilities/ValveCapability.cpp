#include "Contracts/Capabilities/ValveCapability.h"

namespace iotsmartsys::core
{
    ValveCapability::ValveCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapter, event_sink, capability_name, VALVE_ACTUATOR_TYPE, VALVE_STATE_CLOSED)
    {
    }

    void ValveCapability::setup()
    {
        ICommandCapability::setup();
    }

    void ValveCapability::handle()
    {
        std::string hwState = command_hardware_adapter.getState();
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
