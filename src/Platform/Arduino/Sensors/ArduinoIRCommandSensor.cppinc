#ifdef IR_REMOTE_ESP8266
#include <Arduino.h>

#include "ArduinoIRCommandSensor.h"
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include "Contracts/Logging/Log.h"
#include <string>

namespace iotsmartsys::platform::arduino
{
    ArduinoIRCommandSensor::ArduinoIRCommandSensor(int pin)
        : lastCommand(*(new iotsmartsys::core::IRCommand{false, 0, ""})), irPin(pin), lastState(0), lastSendEvent(0), lastStateReadMillis_(0)
    {
        irrecv = new IRrecv(irPin);
    }

    ArduinoIRCommandSensor::~ArduinoIRCommandSensor()
    {
        delete irrecv;
    }

    void ArduinoIRCommandSensor::setup()
    {
        if (irrecv)
        {
            irrecv->enableIRIn();
        }
    }

    void ArduinoIRCommandSensor::handle()
    {
        if (!irrecv)
            return;

        if (irrecv->decode(&results))
        {

            core::Log::get().debug("Código recebido: 0x");
            uint64_t currentValue = results.value;

            core::Log::get().debug("Código recebido: 0x");
            // Format the value as hexadecimal without using Arduino::String
            {
                char buf[32];
                // Use unsigned long long to be safe for 64-bit values
                snprintf(buf, sizeof(buf), "%llX", (unsigned long long)currentValue);
                core::Log::get().debug(buf);
            }

            core::Log::get().debug("resultToSourceCode: 0X");
            lastState = currentValue;

            core::Log::get().debug("resultToHumanReadableBasic: ", resultToHumanReadableBasic(&results));

            bool stateChanged = (currentValue != lastCommand.code) || !lastCommand.triggered;
            lastCommand.triggered = true;
            lastCommand.code = currentValue;
            if (stateChanged)
            {
                lastStateReadMillis_ = millis();
            }

            irrecv->resume();
        }
    }

    iotsmartsys::core::IRCommand ArduinoIRCommandSensor::readCommand() const
    {
        return lastCommand;
    }

    void ArduinoIRCommandSensor::readed()
    {
        lastCommand.triggered = false;
    }

    long ArduinoIRCommandSensor::lastStateReadMillis() const
    {
        return lastStateReadMillis_;
    }

} // namespace iotsmartsys::platform::arduino
#endif
