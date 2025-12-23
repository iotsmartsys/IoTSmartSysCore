#pragma once

#include "IInputCapability.h"

namespace iotsmartsys::core
{
    class PushButtonCapability : public IInputCapability
    {
    public:
        // toleranceTime in milliseconds for debouncing / event grouping
        PushButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);
        PushButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);

        void handle() override;

        bool isPressed() const;

    private:
        unsigned long toleranceTimeMs;
        bool lastState{false};
        unsigned long lastChangeTs{0};
    };

} // namespace iotsmartsys::core
