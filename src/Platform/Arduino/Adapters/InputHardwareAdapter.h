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

        bool applyCommand(const iotsmartsys::core::IHardwareCommand &command) override
        {
            // Input adapter does not support commands
            return false;
        }

        bool applyCommand(const std::string &value) override
        {
            // Input adapter does not support commands
            return false;
        }

        std::string getState() override
        {
            return std::to_string(readInput());
        }

    private:
        int pin;
    };
} // namespace iotsmartsys::platform::arduino
