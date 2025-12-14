// Platform/Arduino/ArduinoTimeProvider.h
#pragma once

#include <Arduino.h>
#include "Contracts/Adapters/IHardwareAdapter.h"
#include "HardwareDigitalLogic.h"
#include "Contracts/Capabilities/ICapabilityType.h"

namespace iotsmartsys::platform::arduino
{

    class OutputHardwareAdapter : public core::IHardwareAdapter
    {
    public:
        OutputHardwareAdapter(int pin, HardwareDigitalLogic logic)
            : pin(pin), logic(logic)
        {
            relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
        }

        void setup() override
        {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, relayState);
        }

        bool applyCommand(const core::IHardwareCommand &command) override
        {
            if (command.command == SWITCH_STATE_ON)
            {
                relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
            }
            else if (command.command == SWITCH_STATE_OFF)
            {
                relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
            }

            updateHardware();
            return true;
        }

        bool applyCommand(const std::string &value) override
        {
            if (value == SWITCH_STATE_ON)
            {
                relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
            }
            else if (value == SWITCH_STATE_OFF)
            {
                relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
            }
            else
            {
                Serial.println("Comando inválido recebido: " + String(value.c_str()));
                return false; // Comando inválido
            }

            updateHardware();
            return true;
        }

        std::string getState() override
        {
            return (digitalRead(pin) == ((logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW)) ? SWITCH_STATE_ON : SWITCH_STATE_OFF;
        }

    private:
        int pin;
        int relayState;
        HardwareDigitalLogic logic;

        void updateHardware()
        {
            Serial.print("Atualizando estado do relé no pino ");
            digitalWrite(pin, relayState);
        }
    };

} // namespace iotsmartsys::platform::arduino
