#include "Contracts/Capabilities/SwitchCapability.h"

namespace iotsmartsys::core
{
    SwitchCapability::SwitchCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapter, event_sink, capability_name, SWITCH_TYPE, SWITCH_STATE_OFF)
    {
    }
    void SwitchCapability::setup()
    {
        ICommandCapability::setup();
    }

    void SwitchCapability::handle()
    {
        std::string hwState = command_hardware_adapter.getState();
        if (hwState != value)
        {
            updateState(hwState);
        }
    }

    void SwitchCapability::toggle()
    {
        if (isOn())
            turnOff();
        else
            turnOn();
    }

    void SwitchCapability::turnOn()
    {
        applyCommand(CapabilityCommand{type.c_str(), SWITCH_STATE_ON});
    }

    void SwitchCapability::turnOff()
    {
        applyCommand(CapabilityCommand{type.c_str(), SWITCH_STATE_OFF});
    }

    bool SwitchCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }

    void SwitchCapability::power(const char *state)
    {
        applyCommand(CapabilityCommand{type.c_str(), state});
    }

} // namespace iotsmartsys::core
