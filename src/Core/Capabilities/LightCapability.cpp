#include "Contracts/Capabilities/LightCapability.h"

namespace iotsmartsys::core
{
    LightCapability::LightCapability(std::string name,
                                     IHardwareAdapter &hardwareAdapter)
        : ICapability(&hardwareAdapter, name, LIGHT_ACTUATOR_TYPE, SWITCH_STATE_OFF)
    {
    }

    void LightCapability::handle()
    {
    }

    void LightCapability::toggle()
    {
        if (isOn())
        {
            turnOff();
        }
        else
        {
            turnOn();
        }
    }

    void LightCapability::turnOn()
    {
        power(SWITCH_STATE_ON);
    }

    void LightCapability::turnOff()
    {
        power(SWITCH_STATE_OFF);
    }

    bool LightCapability::isOn() const
    {
        return value == SWITCH_STATE_ON;
    }

    void LightCapability::executeCommand(const std::string &state)
    {
        power(state);
    }

    void LightCapability::power(const std::string &state)
    {
        applyCommand(iotsmartsys::core::ICommand{type, state});
    }
} // namespace iotsmartsys::core