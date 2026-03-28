#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IIRCommandSensor.h"
#include "Contracts/Interpreters/IRCommandInterpreter.h"

namespace iotsmartsys::core
{
    class AirConditionerCapability : public ICapability
    {
    public:
        AirConditionerCapability(IIRCommandSensor &sensor, IRCommandInterpreter &interpreter, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

    private:
        IIRCommandSensor &sensor;
        IRCommandInterpreter &interpreter;
    };

} // namespace iotsmartsys::core
