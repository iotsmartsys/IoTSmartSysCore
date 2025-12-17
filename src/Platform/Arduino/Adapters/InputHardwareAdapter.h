#pragma once

#include <Arduino.h>
#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::platform::arduino
{
    class InputHardwareAdapter : public iotsmartsys::core::IInputHardwareAdapter
    {
    public:
        InputHardwareAdapter(int pin) : pin(pin) {}

        void setup() override
        {
            pinMode(pin, INPUT);
        }

        int32_t readInput() override
        {
            return static_cast<int32_t>(analogRead(pin));
        }

        bool digitalActive() override
        {
            return digitalRead(pin) == HIGH;
        }

    private:
        int pin;
    };
} // namespace iotsmartsys::platform::arduino
