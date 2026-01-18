#pragma once

#include "Contracts/Capabilities/IInputCapability.h"
#include "Contracts/Capabilities/ICommandCapability.h"
#include <string>

namespace iotsmartsys::core
{
    // Debounce for digital inputs that map to two states (pressed / not pressed, touched / not touched).
    class DebouncedDigitalCapability : public IInputCapability
    {
    public:
        DebouncedDigitalCapability(IInputHardwareAdapter &input_hardware_adapter,
                                   ICapabilityEventSink *event_sink,
                                   const char *capability_name,
                                   const char *type,
                                   const char *initial_value,
                                   unsigned long toleranceTimeMs)
            : IInputCapability(input_hardware_adapter, event_sink, capability_name, type, initial_value),
              _toleranceTimeMs(toleranceTimeMs),
              _lastState(false),
              _lastChangeTs(0)
        {
        }

    protected:
        // Returns true when a debounced state transition was published.
        bool updateDebounced(bool currentState, const char *onValue, const char *offValue)
        {
            const auto now = timeProvider.nowMs();
            if (currentState != _lastState)
            {
                if (now - _lastChangeTs >= _toleranceTimeMs)
                {
                    _lastState = currentState;
                    _lastChangeTs = now;
                    updateState(currentState ? onValue : offValue);
                    return true;
                }
            }
            else
            {
                _lastChangeTs = now;
            }
            return false;
        }

        bool lastState() const { return _lastState; }

    private:
        unsigned long _toleranceTimeMs;
        bool _lastState;
        unsigned long _lastChangeTs;
    };

    // Latches a "triggered" state until timeout elapses with no trigger events.
    class LatchedTriggerCapability : public IInputCapability
    {
    public:
        LatchedTriggerCapability(IInputHardwareAdapter &input_hardware_adapter,
                                 ICapabilityEventSink *event_sink,
                                 const char *capability_name,
                                 const char *type,
                                 const char *initial_value,
                                 unsigned long toleranceTimeMs)
            : IInputCapability(input_hardware_adapter, event_sink, capability_name, type, initial_value),
              _toleranceTimeMs(toleranceTimeMs),
              _latched(false),
              _lastLatchedTs(0)
        {
        }

    protected:
        // Returns true when latched state changed and was published.
        bool updateLatched(bool triggered, const char *onValue, const char *offValue)
        {
            if (triggered)
            {
                _latched = true;
                _lastLatchedTs = timeProvider.nowMs();
            }
            else if (timeProvider.nowMs() - _lastLatchedTs > _toleranceTimeMs)
            {
                _latched = false;
            }

            if (_latched != _lastPublished)
            {
                updateState(_latched ? onValue : offValue);
                _lastPublished = _latched;
                return true;
            }
            return false;
        }

        bool latched() const { return _latched; }
        long lastLatchedAgoMs() const { return timeProvider.nowMs() - _lastLatchedTs; }

    private:
        unsigned long _toleranceTimeMs;
        bool _latched;
        bool _lastPublished{false};
        unsigned long _lastLatchedTs;
    };

    // Binary command capability with common toggle/on/off/power and hardware sync.
    class BinaryCommandCapability : public ICommandCapability
    {
    public:
        BinaryCommandCapability(ICommandHardwareAdapter &hardwareAdapter,
                                ICapabilityEventSink *event_sink,
                                const char *capability_name,
                                const char *type,
                                const char *offValue,
                                const char *onValue)
            : ICommandCapability(hardwareAdapter, event_sink, capability_name, type, offValue),
              _offValue(offValue), _onValue(onValue)
        {
        }

        void handle() override
        {
            syncFromHardware();
        }

        void toggle()
        {
            if (isOn())
                turnOff();
            else
                turnOn();
        }

        void turnOn()
        {
            applyCommand(CapabilityCommand{type.c_str(), _onValue.c_str()});
        }

        void turnOff()
        {
            applyCommand(CapabilityCommand{type.c_str(), _offValue.c_str()});
        }

        bool isOn() const
        {
            return value == _onValue;
        }

        void power(const char *state)
        {
            applyCommand(CapabilityCommand{type.c_str(), state});
        }

    protected:
        void syncFromHardware()
        {
            std::string hwState;
            if (command_interpreter)
            {
                IHardwareState hwStateObj = command_hardware_adapter.getState();
                hwState = command_interpreter->interpretState(hwStateObj);
            }
            else
            {
                hwState = command_hardware_adapter.getStateValue();
            }

            if (hwState != value)
            {
                updateState(hwState);
            }
        }

    private:
        std::string _offValue;
        std::string _onValue;
    };

    // Helper for polling numeric sensors with interval and minimal change tolerance.
    class PollingFloatCapability : public ICapability
    {
    public:
        PollingFloatCapability(ICapabilityEventSink *event_sink,
                               const char *capability_name,
                               const char *type,
                               const char *initial_value,
                               unsigned long readIntervalMs,
                               float minDelta,
                               uint8_t precision)
            : ICapability(event_sink, capability_name, type, initial_value),
              _readIntervalMs(readIntervalMs),
              _minDelta(minDelta),
              _precision(precision)
        {
        }

    protected:
        bool shouldRead(unsigned long now) const
        {
            return (now - _lastReadTime) >= _readIntervalMs;
        }

        bool publishIfChanged(float newValue)
        {
            if (newValue != newValue) // NaN check
                return false;

            if (!_hasValue || (newValue - _lastValue) >= _minDelta)
            {
                _lastValue = newValue;
                _hasValue = true;
                updateState(formatValue(newValue));
                _lastReadTime = static_cast<unsigned long>(timeProvider.nowMs());
                return true;
            }
            _lastReadTime = static_cast<unsigned long>(timeProvider.nowMs());
            return false;
        }

        std::string formatValue(float value) const
        {
            return std::to_string(value);
        }

        void forceNextReadAt(unsigned long now) { _lastReadTime = now; }
        float lastValue() const { return _lastValue; }

    private:
        unsigned long _readIntervalMs;
        float _minDelta;
        uint8_t _precision;
        mutable unsigned long _lastReadTime{0};
        float _lastValue{0.0f};
        bool _hasValue{false};
    };

} // namespace iotsmartsys::core
