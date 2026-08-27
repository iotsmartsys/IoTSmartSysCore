#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Sensors/VoltageSensorTypes.h"

namespace iotsmartsys::core
{
    struct IVoltageSensor : public IHardwareAdapter
    {
        virtual const VoltageMeasurement &voltageMeasurement() const = 0;
    };
}
