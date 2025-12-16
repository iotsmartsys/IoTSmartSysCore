#include "Contracts/Capabilities/HumiditySensorCapability.h"

namespace iotsmartsys::core
{

    HumiditySensorCapability::HumiditySensorCapability(IHumiditySensor &sensor)
        : ICapability(&sensor, HUMIDITY_SENSOR_TYPE, "0"),
          sensor(&sensor),
          humidity(0.0f),
          lastReadTime(0),
          readIntervalMs(60000) // default to 1 minute
    {
    }

    void HumiditySensorCapability::handle()
    {
        unsigned long currentTime = timeProvider.nowMs();
        if (currentTime - lastReadTime < readIntervalMs && humidity > 0)
            return;

        lastReadTime = currentTime;
        float newHumidity = sensor->getHumidityPercentage();

        if (isValidHumidity(newHumidity))
        {
            humidity = newHumidity;
            // publish new state
            updateState(std::to_string(humidity));
        }
    }

    float HumiditySensorCapability::getHumidity() const
    {
        return humidity;
    }

    bool HumiditySensorCapability::isValidHumidity(float hum) const
    {
        return hum >= 0.0f && hum <= 100.0f;
    }

} // namespace iotsmartsys::core