#pragma once

#include <Arduino.h>
#include "Contracts/Adapters/IInputHardwareAdapter.h"
#include "HardwareDigitalLogic.h"

namespace iotsmartsys::platform::arduino
{
    using namespace iotsmartsys::core;
    class InputHardwareAdapter : public iotsmartsys::core::IInputHardwareAdapter
    {
    public:
        InputHardwareAdapter(int pin, HardwareDigitalLogic mode = HardwareDigitalLogic::LOW_IS_ON) : pin(pin), mode(mode) {}
        InputHardwareAdapter(int pin, HardwareDigitalLogic mode, InputPullMode pullMode) : pin(pin), mode(mode), pullMode(pullMode) {}

        void setup() override
        {
            switch (pullMode)
            {
            case InputPullMode::PULL_UP:
                pinMode(pin, INPUT_PULLUP);
                break;
            case InputPullMode::PULL_DOWN:
                pinMode(pin, INPUT_PULLDOWN);
                break;
            default:
                pinMode(pin, INPUT);
                break;
            }
        }

        void setPullMode(InputPullMode mode)
        {
            switch (mode)
            {
            case InputPullMode::PULL_UP:
                digitalWrite(pin, HIGH);
                break;
            case InputPullMode::PULL_DOWN:
                digitalWrite(pin, LOW);
                break;
            default:
                break;
            }
        }

        int32_t readInput() override
        {
            return static_cast<int32_t>(analogRead(pin));
        }

        bool digitalActive() override
        {
            return digitalRead(pin) == ((mode == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW);
        }

        long lastStateReadMillis() const override
        {
            return lastStateReadMillis_;
        }

        void handle() override
        {
            if (digitalRead(pin) != lastState_)
            {
                lastState_ = digitalRead(pin);
                lastStateReadMillis_ = millis();
            }
        }

    private:
        long lastStateReadMillis_{0};
        int lastState_{-1};
        int pin;

        InputPullMode pullMode = InputPullMode::NONE;
        HardwareDigitalLogic mode;
    };
} // namespace iotsmartsys::platform::arduino
