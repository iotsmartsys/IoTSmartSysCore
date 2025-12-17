#include "Contracts/Capabilities/PushButtonCapability.h"

namespace iotsmartsys::core
{
    PushButtonCapability::PushButtonCapability(IInputHardwareAdapter *input_hardware_adapter, ICapabilityEventSink *event_sink, unsigned long toleranceTimeMs)
        : IInputCapability(input_hardware_adapter, event_sink, PUSH_BUTTON_TYPE, PUSH_BUTTON_NO_PRESSED),
          toleranceTimeMs(toleranceTimeMs),
          lastState(false),
          lastChangeTs(0)
    {
    }

    void PushButtonCapability::setup()
    {
        ICapability::setup();
    }

    void PushButtonCapability::handle()
    {
        bool current = inputHardwareAdapter->digitalActive();
        auto now = timeProvider.nowMs();
        if (current != lastState)
        {
            // debounce: require stable state for toleranceTimeMs
            if (now - lastChangeTs >= toleranceTimeMs)
            {
                lastState = current;
                lastChangeTs = now;
                updateState(current ? BUTTON_PRESSED : PUSH_BUTTON_NO_PRESSED);
            }
        }
        else
        {
            lastChangeTs = now;
        }
    }

    bool PushButtonCapability::isPressed() const
    {
        return lastState;
    }

} // namespace iotsmartsys::core
