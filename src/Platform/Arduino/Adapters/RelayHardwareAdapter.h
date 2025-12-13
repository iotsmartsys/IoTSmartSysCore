// Platform/Arduino/ArduinoTimeProvider.h
#pragma once

#include <Arduino.h>
#include "Contracts/Core/Adapters/IHardwareAdapter.h"
#include "HardwareDigitalLogic.h"
#include "Contracts/Core/Capabilities/ICapabilityType.h"
namespace iotsmartsys::platform::arduino
{

    class RelayHardwareAdapter : public core::IHardwareAdapter
    {
    public:
        RelayHardwareAdapter(int pin, HardwareDigitalLogic logic)
            : relayPin(pin), logic(logic)
        {
            relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
        }

        void setup() override
        {
            pinMode(relayPin, OUTPUT);
            digitalWrite(relayPin, relayState);
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
            return (digitalRead(relayPin) == ((logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW)) ? SWITCH_STATE_ON : SWITCH_STATE_OFF;
        }

    private:
        int relayPin;
        int relayState;
        HardwareDigitalLogic logic;

        void updateHardware()
        {
            Serial.print("Atualizando estado do relé no pino ");
            digitalWrite(relayPin, relayState);
        }
    };

} // namespace iotsmartsys::platform::arduino
