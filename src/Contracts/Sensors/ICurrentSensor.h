#pragma once

#include <optional>

#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Sensors/CurrentSensorTypes.h"

namespace iotsmartsys::core
{
    struct ICurrentSensor : public IHardwareAdapter
    {
        virtual const CurrentMeasurement &currentMeasurement() const = 0;
        virtual std::optional<float> calibratedZeroAdcMv() const = 0;
        virtual void requestZeroCalibration() = 0;
    };
}
