#pragma once

#include "IInputCapability.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::core
{
    class ClapSensorCapability : public IInputCapability
    {
    public:
        ClapSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeSeconds);
    ClapSensorCapability(const char *capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, int toleranceTimeSeconds);

        void setup() override;
        void handle() override;

        bool isClapDetected() const;

    private:
        long lastTimeClapDetected;
        bool clapDetected;
        bool lastState;
        int timeTolerance; // milliseconds

        bool isTriggered() const;
        long getTimeSinceLastClapDetected() const;
    };

} // namespace iotsmartsys::core
