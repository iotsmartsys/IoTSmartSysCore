
#pragma once

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "Contracts/Sensors/IIRCommandSensor.h"

#if defined(IR_REMOTE_ESP8266_ENABLED) && IR_REMOTE_ESP8266_ENABLED == 1
#include <IRrecv.h>
#endif

namespace iotsmartsys::platform::arduino
{
#if defined(IR_REMOTE_ESP8266_ENABLED) && IR_REMOTE_ESP8266_ENABLED == 1
    class ArduinoIRCommandSensor : public iotsmartsys::core::IIRCommandSensor
    {
    public:
        ArduinoIRCommandSensor(int pin);
        virtual ~ArduinoIRCommandSensor();

        void setup() override;
        void handle() override;

        iotsmartsys::core::IRCommand &readCommand() override { return lastCommand; }
        void readed() override;
        long lastStateReadMillis() const override;

    private:
        iotsmartsys::core::IRCommand &lastCommand;

        IRrecv *irrecv;
        decode_results results;
        int irPin;
        uint64_t lastState;
        long lastSendEvent;
        long lastStateReadMillis_{0};
    };
#else
    // Stub implementation to keep builds working when IR support is disabled.
    class ArduinoIRCommandSensor : public iotsmartsys::core::IIRCommandSensor
    {
    public:
        explicit ArduinoIRCommandSensor(int) {}
        void setup() override {}
        void handle() override {}
        iotsmartsys::core::IRCommand &readCommand() override { return stubCommand; }
        void readed() override {}
        long lastStateReadMillis() const override { return 0; }

    private:
        iotsmartsys::core::IRCommand stubCommand{false, 0, ""};
    };
#endif

} // namespace iotsmartsys::platform::arduino
