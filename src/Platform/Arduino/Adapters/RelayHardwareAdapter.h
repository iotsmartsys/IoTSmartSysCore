// Platform/Arduino/ArduinoTimeProvider.h

#include <Arduino.h>
#include "Contracts/Core/Adapters/IHardwareAdapater.h"
#include "HardwareDigitalLogic.h"
#include "Contracts/Core/Capabilities/ICapabilityType.h"

namespace iotsmartsys::platform::arduino
{

    class RelayHardwareAdapter : public core::IHardwareAdapater
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

        bool applyCommand(const std::string &command, const std::string &value) override
        {

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
                return false; // Comando inválido
            }

            return true;
        }

        std::string getState() override
        {
            return (relayState == ((logic == HardwareDigitalLogic::HIGH_IS_ON) ? HIGH : LOW)) ? SWITCH_STATE_ON : SWITCH_STATE_OFF;
        }

    private:
        int relayPin;
        int relayState;
        HardwareDigitalLogic logic;
    };

} // namespace iotsmartsys::platform::arduino