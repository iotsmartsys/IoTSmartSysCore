#include "Contracts/Capabilities/ClapSensorCapability.h"

namespace iotsmartsys::core
{
    ClapSensorCapability::ClapSensorCapability(IInputHardwareAdapter *input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeSeconds)
        : ICapability(event_sink, CLAP_SENSOR_TYPE, CLAP_NO_DETECTED), inputHardwareAdapter(input_hardware_adapter),
          lastTimeClapDetected(0), clapDetected(false), lastState(false), timeTolerance(toleranceTimeSeconds * 1000)
    {
    }

    void ClapSensorCapability::handle()
    {
        if (isTriggered())
        {
            clapDetected = true;
            lastTimeClapDetected = timeProvider.nowMs();
        }
        else if (getTimeSinceLastClapDetected() > timeTolerance)
        {
            clapDetected = false;
        }

        if (lastState != clapDetected)
        {
            updateState(clapDetected ? CLAP_DETECTED : CLAP_NO_DETECTED);
            lastState = clapDetected;
        }
    }

    bool ClapSensorCapability::isTriggered() const
    {
        return inputHardwareAdapter->digitalActive();
    }

    bool ClapSensorCapability::isClapDetected() const
    {
        return clapDetected;
    }

    long ClapSensorCapability::getTimeSinceLastClapDetected() const
    {
        return timeProvider.nowMs() - lastTimeClapDetected;
    }

} // namespace iotsmartsys::core
