#include "Contracts/Capabilities/AirConditionerCapability.h"

namespace iotsmartsys::core
{
    AirConditionerCapability::AirConditionerCapability(IIRCommandSensor &sensor, IAirConditionerInterpreter &interpreter, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, AIR_CONDITIONER_TYPE, "OFF"), sensor(sensor), interpreter(interpreter)
    {
        // Initialization code if needed
    }

    void AirConditionerCapability::handle()
    {
        sensor.handle();

        IRCommand command = sensor.readCommand();
        if (command.triggered)
        {
            sensor.readed();
            std::string interpretedValue = interpreter.interpret(command);
            updateState(interpretedValue);
            logger.info("AirConditionerCapability state updated to: %s", interpretedValue.c_str());
        }
    }

} // namespace iotsmartsys::core