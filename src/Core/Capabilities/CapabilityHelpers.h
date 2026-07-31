#pragma once

#include "Contracts/Capabilities/IInputCapability.h"
#include "Contracts/Capabilities/ICommandCapability.h"
#include "Contracts/Providers/ServiceProvider.h"
#include <cmath>
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

        // BCS-009/BCS-010/BCS-011: adapter is initialized first, then a valid
        // persisted record is applied and confirmed by read-back before the
        // capability is considered initialized; absence/invalidity preserves
        // the default flow.
        void setup() override
        {
            command_hardware_adapter.setup();
            restoreFromStorage();
        }

        void handle() override
        {
            syncFromHardware();
        }

        void toggle()
        {
            applyCommand(CapabilityCommand{type.c_str(), TOGGLE_COMMAND});
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
                // BCS-013/BCS-016: every confirmed transition (remote command,
                // public API, adapter-observed sync, derived-class automation
                // such as blink) funnels through this single point.
                persistIfTransition(hwState);
            }
        }

    private:
        providers::IBinaryCapabilityStateProvider *stateStorage() const
        {
            return ServiceProvider::instance().getBinaryCapabilityStateProvider();
        }

        // BCS-009/BCS-011/BCS-012/BCS-020: consult the boot cache by identity
        // (capability_name, type); apply and confirm by read-back before
        // treating the restored value as the logical state. Absence, an
        // identity mismatch or a rejected/unconfirmed command preserve the
        // default flow untouched, without persisting anything (BCS-011).
        void restoreFromStorage()
        {
            auto *storage = stateStorage();
            bool restoredOn = false;
            if (storage && !capability_name.empty() &&
                storage->tryGet(capability_name.c_str(), type.c_str(), restoredOn))
            {
                const std::string &target = restoredOn ? _onValue : _offValue;
                // BCS-004/5.3: restoration must go through the same
                // interpreted path as a normal command, so ValveCapability's
                // on/off <-> open/closed conversion applies here too.
                bool accepted;
                if (command_interpreter)
                {
                    IHardwareCommand hwCommand = command_interpreter->interpretCommand(
                        CapabilityCommand{type.c_str(), target.c_str()});
                    accepted = command_hardware_adapter.applyCommand(hwCommand);
                }
                else
                {
                    accepted = command_hardware_adapter.applyCommand(target.c_str());
                }

                if (accepted)
                {
                    std::string confirmed;
                    if (command_interpreter)
                    {
                        IHardwareState hwStateObj = command_hardware_adapter.getState();
                        confirmed = command_interpreter->interpretState(hwStateObj);
                    }
                    else
                    {
                        confirmed = command_hardware_adapter.getStateValue();
                    }

                    if (confirmed == target)
                    {
                        updateState(confirmed);
                        logger.info("BinaryCommandCapability", "Restored '%s' to '%s'.", capability_name.c_str(), confirmed.c_str());
                        return;
                    }
                    logger.warn("BinaryCommandCapability", "Restore of '%s' not confirmed by adapter; keeping default.", capability_name.c_str());
                }
                else
                {
                    logger.warn("BinaryCommandCapability", "Adapter rejected restore command for '%s'; keeping default.", capability_name.c_str());
                }
            }

            updateState(command_hardware_adapter.getStateValue());
        }

        // BCS-013/BCS-014/BCS-015/BCS-017/BCS-018: persists only the two
        // semantic states (never the transitory "toggle" command), skips
        // writes that would repeat the already-persisted value, and never
        // reverts an already-applied hardware/logical state on storage failure.
        void persistIfTransition(const std::string &confirmedValue)
        {
            if (confirmedValue != _onValue && confirmedValue != _offValue)
                return;

            auto *storage = stateStorage();
            if (!storage || capability_name.empty())
                return;

            const bool newIsOn = (confirmedValue == _onValue);
            const auto result = storage->save(capability_name.c_str(), type.c_str(), newIsOn);
            if (result != common::StateResult::Ok)
            {
                logger.error("BinaryCommandCapability", "Persist failed for '%s' (rc=%d); hardware/logical state unaffected.",
                             capability_name.c_str(), static_cast<int>(result));
            }
        }

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

            if (!_hasValue || std::fabs(newValue - _lastValue) >= _minDelta)
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
