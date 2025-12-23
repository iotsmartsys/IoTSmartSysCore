#pragma once

#include "IInputCapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{

    class DoorSensorCapability : public IInputCapability
    {
    public:
        DoorSensorCapability(IInputHardwareAdapter &input_hardware_adapter,
                             ICapabilityEventSink *event_sink);
        DoorSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink);

        void handle() override;

        bool isOpen();

    private:
        bool lastDoorState;
        bool doorState;
    };
} // namespace iotsmartsys::core