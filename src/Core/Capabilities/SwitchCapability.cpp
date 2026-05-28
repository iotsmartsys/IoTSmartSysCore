#include "Contracts/Capabilities/SwitchCapability.h"

namespace iotsmartsys::core
{
    SwitchCapability::SwitchCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, capability_name, SWITCH_TYPE, STATE_OFF, STATE_ON)
    {
    }

} // namespace iotsmartsys::core
