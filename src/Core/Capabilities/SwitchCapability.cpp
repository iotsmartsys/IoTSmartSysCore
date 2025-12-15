#include "Contracts/Capabilities/SwitchCapability.h"

namespace iotsmartsys::core
{
    SwitchCapability::SwitchCapability(std::string capability_name, IHardwareAdapter &hardwareAdapter)
        : ICapability(&hardwareAdapter, capability_name, SWITCH_TYPE, SWITCH_STATE_OFF)
    {
    }

    void SwitchCapability::setup()
    {
        ICapability::setup();
    }

    void SwitchCapability::handle()
    {
        std::string hwState = hardware_adapator ? hardware_adapator->getState() : std::string(SWITCH_STATE_OFF);
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
        applyCommand(ICommand{type, std::string(SWITCH_STATE_ON)});
    }

    void SwitchCapability::turnOff()
    {
        applyCommand(ICommand{type, std::string(SWITCH_STATE_OFF)});
    }

    bool SwitchCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }

    void SwitchCapability::executeCommand(const std::string &state)
    {
        power(state);
    }

    void SwitchCapability::power(const std::string &state)
    {
        applyCommand(ICommand{type, state});
    }

} // namespace iotsmartsys::core
