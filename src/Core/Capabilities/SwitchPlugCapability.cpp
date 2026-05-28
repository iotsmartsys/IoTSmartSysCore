#include "Contracts/Capabilities/SwitchPlugCapability.h"

namespace iotsmartsys::core
{
    SwitchPlugCapability::SwitchPlugCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, capability_name, SWITCH_PLUG_TYPE, STATE_OFF, STATE_ON)
    {
    }

} // namespace iotsmartsys::core
