#include "Core/Capabilities/CapabilityHelpers.h"
#include "Contracts/Sensors/ITemperatureSensor.h"

namespace iotsmartsys::core
{
    class TemperatureSensorCapability : public PollingFloatCapability
    {
    public:
        TemperatureSensorCapability(ITemperatureSensor &sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs = 60000);
        TemperatureSensorCapability(std::string capability_name, ITemperatureSensor &sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs = 60000);

        void setup() override;
        void handle() override;
        float getTemperature() const;

    private:
        ITemperatureSensor &sensor;

        bool isValidTemperature(float temp) const;
    };

} // namespace iotsmartsys::core
