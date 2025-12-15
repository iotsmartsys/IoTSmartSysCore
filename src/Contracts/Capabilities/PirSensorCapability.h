#pragma once

#include "ICapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{

    class PirSensorCapability : public ICapability
    {
    public:
        PirSensorCapability(IInputHardwareAdapter *input_hardware_adapter, int toleranceTime);

        void handle() override;

        bool isPresenceDetected() const;

    private:
        long lastTimePresenceDetected;
        bool presenceDetected;
        bool lastState;
        int timeTolerance;
        bool pirState;
        IInputHardwareAdapter *inputHardwareAdapter;

        bool isTriggered() const;
        long getTimeSinceLastPresenceDetected() const;
    };
} // namespace iotsmartsys::core