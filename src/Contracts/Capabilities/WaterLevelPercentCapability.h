#pragma once

#include "ICapability.h"
#include "../Providers/ITimeProvider.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"

namespace iotsmartsys::core
{
    class WaterLevelPercentCapability : public ICapability
    {
    public:
        WaterLevelPercentCapability(IWaterLevelSensor *sensor);

        void setup() override;
        void handle() override;
        float getLevelPercent() const;

    private:
        IWaterLevelSensor *sensor{nullptr};
        float lastPercent{0.0f};
    };

} // namespace iotsmartsys::core
