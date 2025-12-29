#pragma once

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
        float readTemperatureCelsius() override;
        float getHumidityPercentage() override;

    private:
        int pin;
        DHT *dht = nullptr;
    };
} // namespace iotsmartsys::platform::arduino
