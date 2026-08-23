#include "Contracts/Capabilities/LEDCapability.h"

namespace iotsmartsys::core
{
    LEDCapability::LEDCapability(const char *capability_name, ICommandHardwareAdapter &hardwareAdapter, ICapabilityEventSink *event_sink)
        : BinaryCommandCapability(hardwareAdapter, event_sink, capability_name, LED_ACTUATOR_TYPE, STATE_OFF, STATE_ON),
          blinkInterval(0), lastToggleTs(0), blinking(false)
    {
    }
    
    void LEDCapability::handle()
    {
        // BCS-016/BCS-AC-015: an override of handle() must not bypass
        // BinaryCommandCapability's read-back, publish and persist protocol;
        // blink only decides whether a new command is issued this cycle and
        // whether that alternation counts as transitory.
        bool transientCycle = false;
        if (blinking && blinkInterval != 0)
        {
            auto now = timeProvider.nowMs();
            if (now - lastToggleTs >= blinkInterval)
            {
                lastToggleTs = now;
                // BCS-DEC-002: produced exclusively by the blink timer.
                transientCycle = true;
                beginTransientCycle();
                if (isOn())
                    BinaryCommandCapability::applyCommand(CapabilityCommand{type.c_str(), STATE_OFF});
                else
                    BinaryCommandCapability::applyCommand(CapabilityCommand{type.c_str(), STATE_ON});
            }
        }

        syncFromHardware();

        if (transientCycle)
        {
            endTransientCycle();
        }
    }

    void LEDCapability::applyCommand(CapabilityCommand command)
    {
        const bool replacesBlink = blinking;
        if (replacesBlink)
        {
            // BCS-DEC-002/BCS-REV-002: every explicit command replaces blink.
            // Timer-owned transitions bypass this override above, so they remain
            // transient and do not terminate their own mode.
            blinking = false;
            blinkInterval = 0;
        }

        BinaryCommandCapability::applyCommand(command);

        if (replacesBlink)
        {
            // Confirm immediately because the current logical value may already
            // equal the requested value. syncFromHardware() persists a changed
            // value; confirmStableState() covers the unchanged case. The common
            // stable-state deduplication guarantees at most one request.
            syncFromHardware();
            confirmStableState();
        }
    }

    void LEDCapability::executeCommand(const char *state) { power(state); }

    void LEDCapability::blink(unsigned long intervalMs)
    {
        if (intervalMs == 0)
        {
            const bool wasBlinking = blinking;
            blinking = false;
            blinkInterval = 0;
            // BCS-DEC-002/BCS-AC-015: leaving blink turns the current confirmed
            // value into a stable state again; it is requested exactly once, and
            // only if it differs from the last stable value requested.
            if (wasBlinking)
            {
                confirmStableState();
            }
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
