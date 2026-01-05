#include "Contracts/Capabilities/TouchButtonCapability.h"

namespace iotsmartsys::core
{
    TouchButtonCapability::TouchButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : DebouncedDigitalCapability(input_hardware_adapter, event_sink, "", BUTTON_TOUCH_TYPE, BUTTON_NO_PRESSED, toleranceTimeMs)
    {
    }

    TouchButtonCapability::TouchButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : DebouncedDigitalCapability(input_hardware_adapter, event_sink, capability_name.c_str(), BUTTON_TOUCH_TYPE, BUTTON_NO_PRESSED, toleranceTimeMs)
    {
    }

    void TouchButtonCapability::handle()
    {
        updateDebounced(inputHardwareAdapter.digitalActive(), BUTTON_PRESSED, BUTTON_NO_PRESSED);
    }

    bool TouchButtonCapability::isTouched() const
    {
        return lastState();
    }

} // namespace iotsmartsys::core
