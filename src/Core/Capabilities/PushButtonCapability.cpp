#include "Contracts/Capabilities/PushButtonCapability.h"

namespace iotsmartsys::core
{
    PushButtonCapability::PushButtonCapability(IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : DebouncedDigitalCapability(input_hardware_adapter, event_sink, "", PUSH_BUTTON_TYPE, PUSH_BUTTON_NO_PRESSED, toleranceTimeMs)
    {
    }

    PushButtonCapability::PushButtonCapability(std::string capability_name, IInputHardwareAdapter &input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : DebouncedDigitalCapability(input_hardware_adapter, event_sink, capability_name.c_str(), PUSH_BUTTON_TYPE, PUSH_BUTTON_NO_PRESSED, toleranceTimeMs)
    {
    }

    void PushButtonCapability::handle()
    {
        logger.debug("PushButton", "Handling state...");
        updateDebounced(inputHardwareAdapter.digitalActive(), BUTTON_PRESSED, PUSH_BUTTON_NO_PRESSED);
    }

    bool PushButtonCapability::isPressed() const
    {
        return lastState();
    }

} // namespace iotsmartsys::core
