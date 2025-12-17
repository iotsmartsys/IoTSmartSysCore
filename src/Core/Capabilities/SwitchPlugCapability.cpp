#include "Contracts/Capabilities/SwitchPlugCapability.h"

namespace iotsmartsys::core
{
    SwitchPlugCapability::SwitchPlugCapability(std::string capability_name, ICommandHardwareAdapter &hardwareAdapter)
        : ICommandCapability(&hardwareAdapter, capability_name, SWITCH_PLUG_TYPE, SWITCH_STATE_OFF)
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
        std::string hwState = command_hardware_adapater ? command_hardware_adapater->getState() : std::string(SWITCH_STATE_OFF);
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
        applyCommand(CapabilityCommand{type, std::string(SWITCH_STATE_ON)});
    }

    void SwitchPlugCapability::turnOff()
    {
        applyCommand(CapabilityCommand{type, std::string(SWITCH_STATE_OFF)});
    }

    bool SwitchPlugCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }

    void SwitchPlugCapability::executeCommand(const std::string &state)
    {
        power(state);
    }

    void SwitchPlugCapability::power(const std::string &state)
    {
        applyCommand(CapabilityCommand{type, state});
    }

} // namespace iotsmartsys::core
