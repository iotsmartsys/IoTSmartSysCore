#pragma once

#include "ICapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{

    class DoorSensorCapability : public ICapability
    {
    public:
        DoorSensorCapability(IInputHardwareAdapter *input_hardware_adapter,
                             ICapabilityEventSink *event_sink);

        void handle() override;

        bool isOpen();

    private:
        bool lastDoorState;
        bool doorState;
        IInputHardwareAdapter *inputHardwareAdapter;
    };
} // namespace iotsmartsys::core