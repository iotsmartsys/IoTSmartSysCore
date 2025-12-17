#include "Contracts/Capabilities/DoorSensorCapability.h"

namespace iotsmartsys::core
{
    DoorSensorCapability::DoorSensorCapability(IInputHardwareAdapter &input_hardware_adapter,
                                               ICapabilityEventSink *event_sink)
        : IInputCapability(input_hardware_adapter, event_sink, DOOR_SENSOR_TYPE, DOOR_SENSOR_OPEN), lastDoorState(0), doorState(false)
    {
    }

    DoorSensorCapability::DoorSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter,
                                               ICapabilityEventSink *event_sink)
        : IInputCapability(input_hardware_adapter, event_sink, capability_name, DOOR_SENSOR_TYPE, DOOR_SENSOR_OPEN), lastDoorState(0), doorState(false)
    {
    }

    void DoorSensorCapability::handle()
    {
        bool currentState = inputHardwareAdapter.digitalActive();
        if (currentState != lastDoorState)
        {
            lastDoorState = currentState;
            this->doorState = currentState;
            updateState(currentState ? DOOR_SENSOR_OPEN : DOOR_SENSOR_CLOSED);
        }
    }

    bool DoorSensorCapability::isOpen()
    {
        return inputHardwareAdapter.digitalActive();
    }
    
    void DoorSensorCapability::setup()
    {
        ICapability::setup();
    }
} // namespace iotsmartsys::core