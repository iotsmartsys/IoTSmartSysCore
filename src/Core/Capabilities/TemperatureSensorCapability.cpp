#include "Contracts/Capabilities/TemperatureSensorCapability.h"
#include <cmath>
#include <cstdio>

namespace iotsmartsys::core
{
    TemperatureSensorCapability::TemperatureSensorCapability(ITemperatureSensor &sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs) : PollingFloatCapability(event_sink, "", TEMPERATURE_SENSOR_TYPE, "0", readIntervalMs, 0.0f, 2), sensor(sensor)
    {
    }

    TemperatureSensorCapability::TemperatureSensorCapability(std::string capability_name, ITemperatureSensor &sensor, ICapabilityEventSink *event_sink, unsigned long readIntervalMs)
        : PollingFloatCapability(event_sink, capability_name.c_str(), TEMPERATURE_SENSOR_TYPE, "0", readIntervalMs, 0.0f, 2), sensor(sensor)
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
        if (!shouldRead(currentTime) && lastValue() > 0.0f)
        {
            logger.debug("TemperatureSensorCapability", "TemperatureSensorCapability: Skipping read, interval not reached.");
            return;
        }

        float temp = sensor.readTemperatureCelsius();
    float roundedTemp = std::round(temp * 100.0f) / 100.0f;
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", roundedTemp);
    const char *tempStr = buf;

        if (isValidTemperature(temp))
        {
            logger.debug("TemperatureSensorCapability", "TemperatureSensorCapability: Valid temperature: %s °C", tempStr);
            publishIfChanged(roundedTemp);
        }
        else
        {
            logger.warn("TemperatureSensorCapability", "TemperatureSensorCapability: Invalid temperature reading: %s °C", tempStr);
            forceNextReadAt(currentTime);
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
