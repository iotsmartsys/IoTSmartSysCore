#include "Contracts/Capabilities/SwitchPlugCapability.h"

namespace iotsmartsys::core
{
    SwitchPlugCapability::SwitchPlugCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapter, event_sink, capability_name, SWITCH_PLUG_TYPE, SWITCH_STATE_OFF)
    {
    }

    void SwitchPlugCapability::setup()
    {
        ICommandCapability::setup();
    }

    void SwitchPlugCapability::handle()
    {
        // no periodic action required; state changes happen via commands or hardware
        // but keep parity with legacy: update internal state from hardware if needed
        std::string hwState = command_hardware_adapter.getState();
        if (hwState != value)
        {
            updateState(hwState);
        }
    }

    void SwitchPlugCapability::toggle()
    {
        if (isOn())
            turnOff();
        else
            turnOn();
    }

    void SwitchPlugCapability::turnOn()
    {
        applyCommand(CapabilityCommand{type.c_str(), SWITCH_STATE_ON});
    }

    void SwitchPlugCapability::turnOff()
    {
        applyCommand(CapabilityCommand{type.c_str(), SWITCH_STATE_OFF});
    }

    bool SwitchPlugCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }
    
    void SwitchPlugCapability::power(const char *state)
    {
        applyCommand(CapabilityCommand{type.c_str(), state});
    }

} // namespace iotsmartsys::core
