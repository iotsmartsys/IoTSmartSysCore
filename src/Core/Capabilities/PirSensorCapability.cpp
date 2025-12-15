#include "Contracts/Capabilities/PirSensorCapability.h"

namespace iotsmartsys::core
{
    PirSensorCapability::PirSensorCapability(IInputHardwareAdapter *input_hardware_adapter, int timeTolerance)
        : ICapability(input_hardware_adapter, PIR_SENSOR_TYPE, PIR_NO_DETECTED), inputHardwareAdapter(input_hardware_adapter),
          lastTimePresenceDetected(0), presenceDetected(false), lastState(false), timeTolerance(timeTolerance)
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
        return inputHardwareAdapter->digitalActive();
    }

    bool PirSensorCapability::isPresenceDetected() const
    {
        return pirState;
    }

    long PirSensorCapability::getTimeSinceLastPresenceDetected() const
    {
        return timeProvider.nowMs() - lastTimePresenceDetected;
    }
    
} // namespace iotsmartsys::core