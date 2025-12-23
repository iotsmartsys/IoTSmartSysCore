#include "Contracts/Capabilities/TouchButtonCapability.h"

namespace iotsmartsys::core
{
    TouchButtonCapability::TouchButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : IInputCapability(input_hardware_adapter, event_sink, BUTTON_TOUCH_TYPE, BUTTON_NO_PRESSED),
          toleranceTimeMs(toleranceTimeMs),
          lastState(false),
          lastChangeTs(0)
    {
    }

    TouchButtonCapability::TouchButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : IInputCapability(input_hardware_adapter, event_sink, capability_name, BUTTON_TOUCH_TYPE, BUTTON_NO_PRESSED), toleranceTimeMs(toleranceTimeMs), lastState(false), lastChangeTs(0)
    {
    }

    void TouchButtonCapability::handle()
    {
        bool current = inputHardwareAdapter.digitalActive();
        auto now = timeProvider.nowMs();
        if (current != lastState)
        {
            if (now - lastChangeTs >= toleranceTimeMs)
            {
                lastState = current;
                lastChangeTs = now;
                updateState(current ? BUTTON_PRESSED : BUTTON_NO_PRESSED);
            }
        }
        else
        {
            lastChangeTs = now;
        }
    }

    bool TouchButtonCapability::isTouched() const
    {
        return lastState;
    }

} // namespace iotsmartsys::core
