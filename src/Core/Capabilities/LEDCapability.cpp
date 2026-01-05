#include "Contracts/Capabilities/LEDCapability.h"

namespace iotsmartsys::core
{
    LEDCapability::LEDCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, capability_name, LED_ACTUATOR_TYPE, LED_STATE_OFF, LED_STATE_ON),
          blinkInterval(0), lastToggleTs(0), blinking(false)
    {
    }
    
    void LEDCapability::handle()
    {
        if (!blinking || blinkInterval == 0)
            return;

        auto now = timeProvider.nowMs();
        if (now - lastToggleTs >= blinkInterval)
        {
            lastToggleTs = now;
            if (isOn())
                turnOff();
            else
                turnOn();
        }
    }

    void LEDCapability::toggle() { BinaryCommandCapability::toggle(); }
    void LEDCapability::turnOn() { BinaryCommandCapability::turnOn(); }
    void LEDCapability::turnOff() { BinaryCommandCapability::turnOff(); }
    bool LEDCapability::isOn() const { return BinaryCommandCapability::isOn(); }
    void LEDCapability::executeCommand(const char *state) { power(state); }

    void LEDCapability::blink(unsigned long intervalMs)
    {
        if (intervalMs == 0)
        {
            blinking = false;
            blinkInterval = 0;
            return;
        }
        blinkInterval = intervalMs;
        blinking = true;
        lastToggleTs = timeProvider.nowMs();
    }

    void LEDCapability::power(const char *state)
    {
        applyCommand(CapabilityCommand{type.c_str(), state});
    }

} // namespace iotsmartsys::core
