#include "Contracts/Capabilities/LightCapability.h"

namespace iotsmartsys::core
{
    LightCapability::LightCapability(const char *name,
                                     ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : ICommandCapability(hardwareAdapter, event_sink, name, LIGHT_ACTUATOR_TYPE, SWITCH_STATE_OFF)
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
        logger.debug("LightCapability", "Turning ON");
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

    void LightCapability::power(const char *state)
    {
        logger.debug("LightCapability", "Setting power state to %s", state);
        ICommandCapability::applyCommand(CapabilityCommand{type.c_str(), state});
    }
} // namespace iotsmartsys::core