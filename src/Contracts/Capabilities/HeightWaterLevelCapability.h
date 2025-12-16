#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"

namespace iotsmartsys::core
{
    class HeightWaterLevelCapability : public ICapability
    {
    public:
        HeightWaterLevelCapability(IWaterLevelSensor *sensor);

        void handle() override;
        float getHeightWaterInCm() const;

    private:
        IWaterLevelSensor *sensor;
        float levelCm;
        float lastLevelCm = 0.0f;
        unsigned long lastCheckMillis = 0;
    };

} // namespace iotsmartsys::core
