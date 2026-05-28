#if defined(IR_REMOTE_ESP8266_ENABLED) && IR_REMOTE_ESP8266_ENABLED == 1
#include <Arduino.h>

#include "ArduinoIRCommandSensor.h"
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include "Contracts/Logging/Log.h"
#include <string>

namespace iotsmartsys::platform::arduino
{
    namespace
    {
        constexpr uint64_t kNecRepeatCode = 0xFFFFFFFFFFFFFFFFULL;
        constexpr uint32_t kCommandReleaseTimeoutMs = 250;
    }

    ArduinoIRCommandSensor::ArduinoIRCommandSensor(int pin)
        : lastCommand(*(new iotsmartsys::core::IRCommand{false, 0, ""})), irPin(pin), lastState(0), activeCommandCode(0), lastSendEvent(0), lastFrameAtMs(0), lastStateReadMillis_(0)
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

        if (!irrecv->decode(&results))
        {
            if (activeCommandCode != 0 &&
                (millis() - lastFrameAtMs) > kCommandReleaseTimeoutMs)
            {
                activeCommandCode = 0;
            }

            return;
        }

        uint64_t currentValue = results.value;
        String decodedType = typeToString(results.decode_type);
        lastFrameAtMs = millis();
        core::Log::get().info("decodedType: ", decodedType.c_str());
        if (results.decode_type == decode_type_t::NEC && currentValue == kNecRepeatCode)
        {
            currentValue = lastState;
        }

        if (currentValue == 0 ||
            (activeCommandCode != 0 && currentValue == activeCommandCode))
        {
            irrecv->resume();
            return;
        }

        core::Log::get().info("Código recebido: 0x");
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "%llX", (unsigned long long)currentValue);
            core::Log::get().info(buf);
        }

        core::Log::get().info("resultToSourceCode: 0X");
        lastState = currentValue;
        activeCommandCode = currentValue;

        core::Log::get().info("resultToHumanReadableBasic: ", resultToHumanReadableBasic(&results));

        bool stateChanged = (currentValue != lastCommand.code) || !lastCommand.triggered;
        lastCommand.triggered = true;
        lastCommand.code = currentValue;
        lastCommand.type = decodedType.c_str();
        if (stateChanged)
        {
            lastStateReadMillis_ = millis();
        }

        irrecv->resume();
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
