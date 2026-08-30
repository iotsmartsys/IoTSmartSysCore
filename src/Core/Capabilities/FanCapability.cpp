#include "Contracts/Capabilities/FanCapability.h"

namespace iotsmartsys::core
{
    FanCapability::FanCapability(const char *capability_name,
                                 ICommandHardwareAdapter &hardwareAdapter,
                                 ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter,
                                  event_sink,
                                  capability_name,
                                  FAN_ACTUATOR_TYPE,
                                  STATE_OFF,
                                  STATE_ON)
    {
    }

} // namespace iotsmartsys::core
