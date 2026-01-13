#include "Contracts/Capabilities/HumiditySensorCapability.h"
#include <cmath>
#include <cstdio>
namespace iotsmartsys::core
{

    HumiditySensorCapability::HumiditySensorCapability(IHumiditySensor &sensor, ICapabilityEventSink *event_sink)
        : PollingFloatCapability(event_sink, "", HUMIDITY_SENSOR_TYPE, "0", 60000, 0.0f, 2),
          sensor(sensor)
    {
    }

    HumiditySensorCapability::HumiditySensorCapability(std::string capability_name, IHumiditySensor &sensor, ICapabilityEventSink *event_sink)
        : PollingFloatCapability(event_sink, capability_name.c_str(), HUMIDITY_SENSOR_TYPE, "0", 60000, 0.0f, 2),
          sensor(sensor)
    {
    }

    void HumiditySensorCapability::setup()
    {
        ICapability::setup();
        sensor.setup();
    }

    void HumiditySensorCapability::handle()
    {
        unsigned long currentTime = timeProvider.nowMs();
        if (!shouldRead(currentTime) && lastValue() > 0.0f)
            return;

        float newHumidity = sensor.getHumidityPercentage();
    float roundedHumidity = std::round(newHumidity * 100.0f) / 100.0f;
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", roundedHumidity);
    const char* humidityStr = buf;

        if (isValidHumidity(newHumidity))
        {
            publishIfChanged(roundedHumidity);
        }
        else
        {
            forceNextReadAt(currentTime);
        }
    }

    float HumiditySensorCapability::getHumidity() const
    {
        return lastValue();
    }

    bool HumiditySensorCapability::isValidHumidity(float hum) const
    {
        return hum >= 0.0f && hum <= 100.0f;
    }

} // namespace iotsmartsys::core
