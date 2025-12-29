// Platform/Arduino/ArduinoTimeProvider.h
#pragma once

#include <Arduino.h>
#include <cstring>
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "HardwareDigitalLogic.h"
#include "Contracts/Capabilities/ICapabilityType.h"

namespace iotsmartsys::platform::arduino
{

    class OutputHardwareAdapter : public core::ICommandHardwareAdapter
    {
    public:
        OutputHardwareAdapter(int pin, HardwareDigitalLogic logic)
            : pin(pin), logic(logic)
        {
            pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
        }

        void setup() override
        {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, pinState);
        }

        bool applyCommand(const core::IHardwareCommand &command) override
        {
            if (command.command == SWITCH_STATE_ON)
            {
                pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
            }
            else if (command.command == SWITCH_STATE_OFF)
            {
                pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
            }
            else if (command.command == TOGGLE_COMMAND)
            {
                if (digitalRead(pin) == HIGH)
                {
                    pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
                }
                else
                {
                    pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
                }
            }
            else
            {
                return false; // Comando inválido
            }

            updateHardware();
            return true;
        }

        // Primary implementation using C-string to match core::ICommandHardwareAdapter
        bool applyCommand(const char *value) override
        {
            if (strcmp(value, SWITCH_STATE_ON) == 0)
            {
                pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
            }
            else if (strcmp(value, SWITCH_STATE_OFF) == 0)
            {
                pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
            }
            else if (strcmp(value, TOGGLE_COMMAND) == 0)
            {
                if (digitalRead(pin) == HIGH)
                {
                    pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
                }
                else
                {
                    pinState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
                }
            }
            else
            {
                return false; // Comando inválido
            }

            updateHardware();
            return true;
        }

        // Backwards-compatible overload accepting std::string
        bool applyCommand(const std::string &value) { return applyCommand(value.c_str()); }

        std::string getState() override
        {
            return (digitalRead(pin) == ((logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW)) ? SWITCH_STATE_ON : SWITCH_STATE_OFF;
        }

    private:
        int pin;
        int pinState;
        HardwareDigitalLogic logic;

        void updateHardware()
        {
            digitalWrite(pin, pinState);
        }
    };

} // namespace iotsmartsys::platform::arduino
