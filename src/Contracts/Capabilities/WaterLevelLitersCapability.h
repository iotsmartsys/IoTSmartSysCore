#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"

namespace iotsmartsys::core
{
    class WaterLevelLitersCapability : public ICapability
    {

    public:
        WaterLevelLitersCapability(IWaterLevelSensor *sensor);

        void handle() override;
        float getLevelLiters();

    private:
        IWaterLevelSensor *sensor;
        float levelLiters;
        float lastLevelLiters = 0.0;
        unsigned long lastCheckMillis = 0;
    };
}