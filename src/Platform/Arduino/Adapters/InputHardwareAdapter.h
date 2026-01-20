#pragma once

#include <Arduino.h>
#include "Contracts/Adapters/IInputHardwareAdapter.h"
#include "Contracts/Logging/Log.h"
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
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
                pinMode(pin, INPUT);
                iotsmartsys::core::Log::get().warn("InputHardwareAdapter", "PULL_DOWN not supported on ESP8266 GPIO; using INPUT (external pulldown required)");
#else
                pinMode(pin, INPUT_PULLDOWN);
#endif
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
