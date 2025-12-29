#include "Contracts/Capabilities/TemperatureSensorCapability.h"
#include <cmath>
#include <iomanip>
#include <sstream>

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
            logger.debug("TemperatureSensorCapability", "TemperatureSensorCapability: Skipping read, interval not reached. Next read in %lu ms", readIntervalMs - (currentTime - lastReadTime));
            return;
        }

        lastReadTime = currentTime;
        float temp = sensor.readTemperatureCelsius();
        float roundedTemp = std::round(temp * 100.0f) / 100.0f;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << roundedTemp;
        std::string tempStr = oss.str();

        if (isValidTemperature(temp))
        {
            logger.debug("TemperatureSensorCapability", "TemperatureSensorCapability: Valid temperature: %s °C", tempStr.c_str());
            temperature = roundedTemp;
            updateState(tempStr);
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
