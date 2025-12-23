#include "Contracts/Capabilities/SwitchPlugCapability.h"

namespace iotsmartsys::core
{
    SwitchPlugCapability::SwitchPlugCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
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
        applyCommand(DeviceCommand{type, std::string(SWITCH_STATE_ON)});
    }

    void SwitchPlugCapability::turnOff()
    {
        applyCommand(DeviceCommand{type, std::string(SWITCH_STATE_OFF)});
    }

    bool SwitchPlugCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }
    
    void SwitchPlugCapability::power(const std::string &state)
    {
        applyCommand(DeviceCommand{type, state});
    }

} // namespace iotsmartsys::core
