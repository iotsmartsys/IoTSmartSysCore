#include "Contracts/Capabilities/DoorSensorCapability.h"

namespace iotsmartsys::core
{
    DoorSensorCapability::DoorSensorCapability(IInputHardwareAdapter *input_hardware_adapter,
                                               ICapabilityEventSink *event_sink)
        : ICapability(event_sink, DOOR_SENSOR_TYPE, DOOR_SENSOR_OPEN), inputHardwareAdapter(input_hardware_adapter), lastDoorState(0), doorState(false)
    {
    }

    void DoorSensorCapability::handle()
    {
        bool currentState = inputHardwareAdapter->digitalActive();
        if (currentState != lastDoorState)
        {
            lastDoorState = currentState;
            this->doorState = currentState;
            updateState(currentState ? DOOR_SENSOR_OPEN : DOOR_SENSOR_CLOSED);
        }
    }

    bool DoorSensorCapability::isOpen()
    {
        return inputHardwareAdapter->digitalActive();
    }
} // namespace iotsmartsys::core