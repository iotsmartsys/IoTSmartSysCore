#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Sensors/PowerSensorTypes.h"

namespace iotsmartsys::core
{
    struct IPowerSensor : public IHardwareAdapter
    {
        virtual const PowerMeasurement &powerMeasurement() const = 0;
    };
}
