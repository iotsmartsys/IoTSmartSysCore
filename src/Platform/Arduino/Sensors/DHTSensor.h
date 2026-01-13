#pragma once

#ifdef DHT_SENSOR_ENABLED

#include "Contracts/Sensors/ITemperatureSensor.h"
#include "Contracts/Sensors/IHumiditySensor.h"
#include <DHT.h>

namespace iotsmartsys::platform::arduino
{
    class DHTSensor : public core::ITemperatureSensor, public core::IHumiditySensor
    {
    public:
        DHTSensor(int pin);
        void setup() override;
        void handle() override;
        float readTemperatureCelsius() override;
        float getHumidityPercentage() override;
        long lastStateReadMillis() const override;

    private:
        int pin;
        DHT *dht = nullptr;
        float lastTemperatureC_{0.0f};
        float lastHumidity_{0.0f};
        long lastStateReadMillis_{0};
        bool hasReading_{false};
    };
} // namespace iotsmartsys::platform::arduino
#endif // DHT_SENSOR_ENABLED