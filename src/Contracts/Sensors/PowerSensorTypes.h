#pragma once

#include <optional>

namespace iotsmartsys::core
{
    enum class PowerMeasurementStatus
    {
        NOT_READY,
        VALID,
        ESTIMATED,
        INPUT_INVALID
    };

    struct PowerMeasurement
    {
        std::optional<double> powerW;
        PowerMeasurementStatus measurementStatus{PowerMeasurementStatus::NOT_READY};
    };
}
