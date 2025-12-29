#pragma once

#include "IInputCapability.h"

namespace iotsmartsys::core
{
    class TouchButtonCapability : public IInputCapability
    {
    public:
    TouchButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);
    TouchButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs = 50);

        void handle() override;

        bool isTouched() const;

    private:
        unsigned long toleranceTimeMs;
        bool lastState{false};
        unsigned long lastChangeTs{0};
    };

} // namespace iotsmartsys::core
