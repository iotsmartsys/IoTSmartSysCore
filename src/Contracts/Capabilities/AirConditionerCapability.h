#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IIRCommandSensor.h"
#include "Contracts/Interpreters/IAirConditionerInterpreter.h"

namespace iotsmartsys::core
{
    class AirConditionerCapability : public ICapability
    {
    public:
        AirConditionerCapability(IIRCommandSensor *sensor, IAirConditionerInterpreter *interpreter, ICapabilityEventSink *event_sink);

        void setup() override;
        void handle() override;

    private:
        IIRCommandSensor *sensor;
        IAirConditionerInterpreter *interpreter;
    };

} // namespace iotsmartsys::core
