#include "Contracts/Capabilities/TemperatureSensorCapability.h"

namespace iotsmartsys::core
{
    TemperatureSensorCapability::TemperatureSensorCapability(ITemperatureSensor *sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs) : ICapability(event_sink, TEMPERATURE_SENSOR_TYPE, "0"), currentTemperature(0.0f), sensor(sensor), readIntervalMs(readIntervalMs)
    {
    }

    void TemperatureSensorCapability::handle()
    {
        unsigned long currentTime = timeProvider.nowMs();
        if (currentTime - lastReadTime < readIntervalMs && temperature > 0)
            return;

        lastReadTime = currentTime;
        float temp = sensor->readTemperatureCelsius();

        if (isValidTemperature(temp))
        {
            temperature = temp;
            updateState(std::to_string(temperature));
        }
    }

    float TemperatureSensorCapability::getTemperature() const
    {
        return sensor->readTemperatureCelsius();
    }

    bool TemperatureSensorCapability::isValidTemperature(float temp) const
    {
        return (temp >= -40.0f && temp <= 125.0f);
    }

} // namespace iotsmartsys::core