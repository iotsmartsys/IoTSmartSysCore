#include "Contracts/Capabilities/PirSensorCapability.h"

namespace iotsmartsys::core
{
    PirSensorCapability::PirSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int timeTolerance)
        : LatchedTriggerCapability(input_hardware_adapter, event_sink, "", PIR_SENSOR_TYPE, PIR_NO_DETECTED, timeTolerance)
    {
    }

    PirSensorCapability::PirSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int timeTolerance)
        : LatchedTriggerCapability(input_hardware_adapter, event_sink, capability_name.c_str(), PIR_SENSOR_TYPE, PIR_NO_DETECTED, timeTolerance)
    {
    }

    void PirSensorCapability::handle()
    {
        const bool changed = updateLatched(isTriggered(), PIR_DETECTED, PIR_NO_DETECTED);
        if (changed)
        {
            logger.debug("PIR", "Presence state changed to: ", latched());
        }
    }

    bool PirSensorCapability::isTriggered() const
    {
        return inputHardwareAdapter.digitalActive();
    }

    bool PirSensorCapability::isPresenceDetected() const
    {
        return latched();
    }
    
} // namespace iotsmartsys::core
