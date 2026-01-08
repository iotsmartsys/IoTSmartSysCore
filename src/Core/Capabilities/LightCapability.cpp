#include "Contracts/Capabilities/LightCapability.h"

namespace iotsmartsys::core
{
    LightCapability::LightCapability(const char *name,
                                     ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, name, LIGHT_ACTUATOR_TYPE, SWITCH_STATE_OFF, SWITCH_STATE_ON)
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
        BinaryCommandCapability::turnOn();
    }
} // namespace iotsmartsys::core
