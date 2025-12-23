// Platform/Arduino/ArduinoTimeProvider.h
#pragma once

#include <Arduino.h>
#include <cstring>
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "HardwareDigitalLogic.h"
#include "Contracts/Capabilities/ICapabilityType.h"
#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{

    class RelayHardwareAdapter : public core::ICommandHardwareAdapter
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

        // Primary implementation using C-string to match core::ICommandHardwareAdapter
        bool applyCommand(const char *value) override
        {
            if (strcmp(value, SWITCH_STATE_ON) == 0)
            {
                relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
            }
            else if (strcmp(value, SWITCH_STATE_OFF) == 0)
            {
                relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
            }
            else if (strcmp(value, TOGGLE_COMMAND) == 0)
            {
                if (digitalRead(relayPin) == HIGH)
                {
                    relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW;
                }
                else
                {
                    relayState = (logic == HardwareDigitalLogic::HIGH_IS_ON) ? LOW : HIGH;
                }
            }
            else
            {
                core::Log::get().error("RelayHardwareAdapter", "Invalid command value: %s", value);
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
            iotsmartsys::core::Log::get().info("Atualizando estado do relé no pino ");
            digitalWrite(relayPin, relayState);
        }
    };

} // namespace iotsmartsys::platform::arduino
