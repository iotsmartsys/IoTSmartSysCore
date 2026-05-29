#include "Contracts/Capabilities/LightCapability.h"

namespace iotsmartsys::core
{
    LightCapability::LightCapability(const char *name,
                                     ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, name, LIGHT_ACTUATOR_TYPE, STATE_OFF, STATE_ON)
    {
    }

    void LightCapability::toggle()
    {
        logger.debug("LightCapability", "Toggling");
        BinaryCommandCapability::toggle();
    }

    void LightCapability::turnOn()
    {
        logger.debug("LightCapability", "Turning ON");
        BinaryCommandCapability::turnOn();
    }
} // namespace iotsmartsys::core
