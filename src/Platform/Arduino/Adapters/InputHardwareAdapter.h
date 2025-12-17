#pragma once

#include <Arduino.h>
#include "Contracts/Adapters/IInputHardwareAdapter.h"
#include "HardwareDigitalLogic.h"

namespace iotsmartsys::platform::arduino
{
    enum class InputPullMode
    {
        NONE = 0,
        UP = 1,
        DOWN = 2
    };

    class InputHardwareAdapter : public iotsmartsys::core::IInputHardwareAdapter
    {
    public:
        InputHardwareAdapter(int pin, HardwareDigitalLogic mode = HardwareDigitalLogic::LOW_IS_ON) : pin(pin), mode(mode) {}
        InputHardwareAdapter(int pin, HardwareDigitalLogic mode, InputPullMode pullMode) : pin(pin), mode(mode), pullMode(pullMode) {}

        void setup() override
        {
            switch (pullMode)
            {
            case InputPullMode::UP:
                pinMode(pin, INPUT_PULLUP);
                break;
            case InputPullMode::DOWN:
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
            case InputPullMode::UP:
                digitalWrite(pin, HIGH);
                break;
            case InputPullMode::DOWN:
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

    private:
        int pin;
        InputPullMode pullMode = InputPullMode::NONE;
        HardwareDigitalLogic mode;
    };
} // namespace iotsmartsys::platform::arduino
