
#pragma once

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "Contracts/Sensors/IIRCommandSensor.h"

#ifdef IR_REMOTE_ESP8266
#include <IRrecv.h>
#endif

namespace iotsmartsys::platform::arduino
{
#ifdef IR_REMOTE_ESP8266
    class ArduinoIRCommandSensor : public iotsmartsys::core::IIRCommandSensor
    {
    public:
        ArduinoIRCommandSensor(int pin);
        virtual ~ArduinoIRCommandSensor();

        void setup() override;
        void handle() override;

        iotsmartsys::core::IRCommand readCommand() const override;
        void readed() override;

    private:
        iotsmartsys::core::IRCommand &lastCommand;

        IRrecv *irrecv;
        decode_results results;
        int irPin;
        uint64_t lastState;
        long lastSendEvent;
    };
#else
    // Stub implementation to keep builds working when IR support is disabled.
    class ArduinoIRCommandSensor : public iotsmartsys::core::IIRCommandSensor
    {
    public:
        explicit ArduinoIRCommandSensor(int) {}
        void setup() override {}
        void handle() override {}
        iotsmartsys::core::IRCommand readCommand() const override { return iotsmartsys::core::IRCommand{false, 0, ""}; }
        void readed() override {}
    };
#endif

} // namespace iotsmartsys::platform::arduino
