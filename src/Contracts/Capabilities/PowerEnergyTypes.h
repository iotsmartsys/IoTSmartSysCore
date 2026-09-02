#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace iotsmartsys::core
{
    enum class PowerEnergyMeasurementStatus
    {
        NOT_READY,
        VALID,
        ESTIMATED,
        INPUT_INVALID
    };

    struct PowerEnergyMeasurement
    {
        std::optional<double> powerW;
        double energyWh{0.0};
        PowerEnergyMeasurementStatus measurementStatus{PowerEnergyMeasurementStatus::NOT_READY};
    };

    struct PowerEnergyConfig
    {
        std::string id;
        std::uint32_t readingIntervalMs{1000};
    };

    inline const char *toString(PowerEnergyMeasurementStatus status)
    {
        switch (status)
        {
        case PowerEnergyMeasurementStatus::NOT_READY:
            return "NOT_READY";
        case PowerEnergyMeasurementStatus::VALID:
            return "VALID";
        case PowerEnergyMeasurementStatus::ESTIMATED:
            return "ESTIMATED";
        case PowerEnergyMeasurementStatus::INPUT_INVALID:
            return "INPUT_INVALID";
        }
        return "NOT_READY";
    }
}
