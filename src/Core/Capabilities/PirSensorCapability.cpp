#include "Contracts/Capabilities/PirSensorCapability.h"

namespace iotsmartsys::core
{
    PirSensorCapability::PirSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int timeTolerance)
        : IInputCapability(input_hardware_adapter, event_sink, PIR_SENSOR_TYPE, PIR_NO_DETECTED),
          lastTimePresenceDetected(0), presenceDetected(false), lastState(false), timeTolerance(timeTolerance)
    {
    }

    PirSensorCapability::PirSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int timeTolerance)
        : IInputCapability(input_hardware_adapter, event_sink, capability_name, PIR_SENSOR_TYPE, PIR_NO_DETECTED), lastTimePresenceDetected(0), presenceDetected(false), lastState(false), timeTolerance(timeTolerance)
    {
    }

    void PirSensorCapability::handle()
    {
        if (isTriggered())
        {

            logger.debug("PIR", "Last State: ", lastState);
            logger.debug("PIR", "PIR Sensor triggered");
            presenceDetected = true;
            lastTimePresenceDetected = timeProvider.nowMs();
        }
        else if (getTimeSinceLastPresenceDetected() > timeTolerance)
        {
            logger.debug("PIR Sensor no longer triggered");
            presenceDetected = false;
        }
        if (lastState != presenceDetected)
        {
            logger.debug("Presence state changed to: ", presenceDetected);
            updateState(presenceDetected ? PIR_DETECTED : PIR_NO_DETECTED);
            lastState = presenceDetected;
        }
    }

    bool PirSensorCapability::isTriggered() const
    {
        return inputHardwareAdapter.digitalActive();
    }

    bool PirSensorCapability::isPresenceDetected() const
    {
        return pirState;
    }

    long PirSensorCapability::getTimeSinceLastPresenceDetected() const
    {
        return timeProvider.nowMs() - lastTimePresenceDetected;
    }
    
    void PirSensorCapability::setup()
    {
        ICapability::setup();
    }
    
} // namespace iotsmartsys::core