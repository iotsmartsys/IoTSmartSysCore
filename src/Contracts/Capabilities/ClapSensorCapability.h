#pragma once

#include "ICapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{
    class ClapSensorCapability : public ICapability
    {
    public:
        ClapSensorCapability(IInputHardwareAdapter *input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeSeconds);

        void handle() override;

        bool isClapDetected() const;

    private:
        long lastTimeClapDetected;
        bool clapDetected;
        bool lastState;
        int timeTolerance; // milliseconds
        IInputHardwareAdapter *inputHardwareAdapter;

        bool isTriggered() const;
        long getTimeSinceLastClapDetected() const;
    };

} // namespace iotsmartsys::core
