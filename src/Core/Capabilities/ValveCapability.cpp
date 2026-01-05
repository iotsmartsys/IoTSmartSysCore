#include "Contracts/Capabilities/ValveCapability.h"

namespace iotsmartsys::core
{
    ValveCapability::ValveCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, capability_name, VALVE_ACTUATOR_TYPE, VALVE_STATE_CLOSED, VALVE_STATE_OPEN)
    {
    }

    void ValveCapability::handle()
    {
        syncFromHardware();
    }

    void ValveCapability::turnOpen()
    {
        turnOn();
    }

    void ValveCapability::turnClosed()
    {
        turnOff();
    }

    bool ValveCapability::isOpen() const
    {
        return isOn();
    }

} // namespace iotsmartsys::core
