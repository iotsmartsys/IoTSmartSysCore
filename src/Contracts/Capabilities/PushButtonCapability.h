#pragma once

#include "IInputCapability.h"

namespace iotsmartsys::core
{
    class PushButtonCapability : public IInputCapability
    {
    public:
        // toleranceTime in milliseconds for debouncing / event grouping
        PushButtonCapability(IInputHardwareAdapter *input_hardware_adapter, unsigned long toleranceTimeMs = 50);

        void setup() override;
        void handle() override;

        bool isPressed() const;

    private:
        unsigned long toleranceTimeMs;
        bool lastState{false};
        unsigned long lastChangeTs{0};
    };

} // namespace iotsmartsys::core
