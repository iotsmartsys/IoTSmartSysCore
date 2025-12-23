#pragma once

#include "IInputCapability.h"

namespace iotsmartsys::core
{
    class ValveCapability; // forward

    class WaterFlowHallSensorCapability : public IInputCapability
    {
    public:
        WaterFlowHallSensorCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink);
        WaterFlowHallSensorCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink);

        void handle() override;

        // called by ISR or test harness to increment pulse count
        void incrementPulseCount();

        float getTotalLiters() const { return totalLiters; }

    private:
        unsigned long lastMillis{0};
        unsigned long pulseCount{0};
        float totalLiters{0.0f};
        float lastTotalLiters{0.0f};
    };

} // namespace iotsmartsys::core
