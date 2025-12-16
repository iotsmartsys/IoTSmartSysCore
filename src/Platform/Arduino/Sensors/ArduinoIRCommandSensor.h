
#pragma once

#include <Arduino.h>
#include "Contracts/Sensors/IIRCommandSensor.h"
#include <IRrecv.h>

namespace iotsmartsys::platform::arduino
{
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

} // namespace iotsmartsys::core
