#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IHumiditySensor.h"

namespace iotsmartsys::core
{
    class HumiditySensorCapability : public ICapability
    {
    public:
        HumiditySensorCapability(IHumiditySensor &sensor);

        void handle() override;
        float getHumidity() const;

    private:
        unsigned long readIntervalMs = 60000;
        float humidity = 0;
        float currentHumidity{0.0f};
        unsigned long lastReadTime = 0;
        IHumiditySensor *sensor;

        bool isValidHumidity(float hum) const;
    };

} // namespace iotsmartsys::core