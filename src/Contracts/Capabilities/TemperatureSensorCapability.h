#include "ICapability.h"
#include "Contracts/Sensors/ITemperatureSensor.h"

namespace iotsmartsys::core
{
    class TemperatureSensorCapability : public ICapability
    {
    public:
        TemperatureSensorCapability(ITemperatureSensor *sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs = 60000);

        void handle() override;
        float getTemperature() const;

    private:
        unsigned long readIntervalMs;
        float temperature = 0;
        float currentTemperature{0.0f};
        unsigned long lastReadTime = 0;
        ITemperatureSensor *sensor;

        bool isValidTemperature(float temp) const;
    };

} // namespace iotsmartsys::core