#include "Contracts/Capabilities/ClapSensorCapability.h"

namespace iotsmartsys::core
{
    ClapSensorCapability::ClapSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeMs)
        : LatchedTriggerCapability(input_hardware_adapter, event_sink, "", CLAP_SENSOR_TYPE, CLAP_NO_DETECTED, toleranceTimeMs)
    {
    }

    ClapSensorCapability::ClapSensorCapability(const char *capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeMs)
        : LatchedTriggerCapability(input_hardware_adapter, event_sink, capability_name, CLAP_SENSOR_TYPE, CLAP_NO_DETECTED, toleranceTimeMs)
    {
    }

    void ClapSensorCapability::handle()
    {
        updateLatched(isTriggered(), CLAP_DETECTED, CLAP_NO_DETECTED);
    }

    bool ClapSensorCapability::isTriggered() const
    {
        return inputHardwareAdapter.digitalActive();
    }

    bool ClapSensorCapability::isClapDetected() const
    {
        return latched();
    }

} // namespace iotsmartsys::core
