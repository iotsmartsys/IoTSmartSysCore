#include "Contracts/Capabilities/SwitchCapability.h"

namespace iotsmartsys::core
{
    SwitchCapability::SwitchCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
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
        applyCommand(DeviceCommand{type, std::string(SWITCH_STATE_ON)});
    }

    void SwitchCapability::turnOff()
    {
        applyCommand(DeviceCommand{type, std::string(SWITCH_STATE_OFF)});
    }

    bool SwitchCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }

    void SwitchCapability::power(const std::string &state)
    {
        applyCommand(DeviceCommand{type, state});
    }

} // namespace iotsmartsys::core
