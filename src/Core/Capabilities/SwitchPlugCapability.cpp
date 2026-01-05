#include "Contracts/Capabilities/SwitchPlugCapability.h"

namespace iotsmartsys::core
{
    SwitchPlugCapability::SwitchPlugCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, capability_name, SWITCH_PLUG_TYPE, SWITCH_STATE_OFF, SWITCH_STATE_ON)
    {
    }

    void SwitchPlugCapability::handle()
    {
        // no periodic action required; state changes happen via commands or hardware
        // but keep parity with legacy: update internal state from hardware if needed
        syncFromHardware();
    }

} // namespace iotsmartsys::core
