#include "Contracts/Capabilities/TemperatureSensorCapability.h"

namespace iotsmartsys::core
{
    TemperatureSensorCapability::TemperatureSensorCapability(ITemperatureSensor &sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs) : ICapability(event_sink, TEMPERATURE_SENSOR_TYPE, "0"), currentTemperature(0.0f), sensor(sensor), readIntervalMs(readIntervalMs)
    {
    }

    TemperatureSensorCapability::TemperatureSensorCapability(std::string capability_name, ITemperatureSensor &sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs)
        : ICapability(event_sink, capability_name, TEMPERATURE_SENSOR_TYPE, "0"), currentTemperature(0.0f), sensor(sensor), readIntervalMs(readIntervalMs)
    {
    }

    void TemperatureSensorCapability::setup()
    {
        ICapability::setup();
        sensor.setup();
    }

    void TemperatureSensorCapability::handle()
    {
        unsigned long currentTime = timeProvider.nowMs();
        if (currentTime - lastReadTime < readIntervalMs && temperature > 0)
        {
            logger.warn("TemperatureSensorCapability", "TemperatureSensorCapability: Skipping read, interval not reached.");
            return;
        }

        lastReadTime = currentTime;
        float temp = sensor.readTemperatureCelsius();
        std::string tempStr = std::to_string(temp);
        logger.info("TemperatureSensorCapability", "TemperatureSensorCapability: Read temperature: %s °C", tempStr.c_str());

        if (isValidTemperature(temp))
        {
            logger.info("TemperatureSensorCapability", "TemperatureSensorCapability: Valid temperature: %s °C", tempStr.c_str());
            temperature = temp;
            updateState(std::to_string(temperature));
        }
        else
        {
            logger.warn("TemperatureSensorCapability", "TemperatureSensorCapability: Invalid temperature reading: %s °C", tempStr.c_str());
        }
    }

    float TemperatureSensorCapability::getTemperature() const
    {
        return sensor.readTemperatureCelsius();
    }

    bool TemperatureSensorCapability::isValidTemperature(float temp) const
    {
        return (temp >= -40.0f && temp <= 125.0f);
    }

} // namespace iotsmartsys::core
