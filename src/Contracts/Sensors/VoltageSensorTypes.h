#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace iotsmartsys::core
{
    enum class VoltageMeasurementStatus
    {
        NOT_READY,
        VALID,
        BELOW_MINIMUM,
        ADC_SATURATION
    };

    enum class VoltageSensorAdcAttenuation
    {
        FULL_RANGE
    };

    struct VoltageMeasurement
    {
        std::optional<float> voltageV;
        VoltageMeasurementStatus measurementStatus{VoltageMeasurementStatus::NOT_READY};
    };

    struct VoltageSensorConfig
    {
        std::string id;
        int adcPin{-1};
        std::uint8_t adcResolutionBits{12};
        VoltageSensorAdcAttenuation adcAttenuation{VoltageSensorAdcAttenuation::FULL_RANGE};
        float adcMinimumMv{144.0f};
        float adcMaximumMv{3100.0f};
        float r1Ohms{0.0f};
        float r2Ohms{0.0f};
        std::uint32_t samplesPerReading{100};
        std::uint32_t sampleIntervalUs{200};
        std::uint32_t readingIntervalMs{500};
        std::uint32_t capabilityEvaluationIntervalMs{1000};
    };

    inline const char *toString(VoltageMeasurementStatus status)
    {
        switch (status)
        {
        case VoltageMeasurementStatus::NOT_READY:
            return "NOT_READY";
        case VoltageMeasurementStatus::VALID:
            return "VALID";
        case VoltageMeasurementStatus::BELOW_MINIMUM:
            return "BELOW_MINIMUM";
        case VoltageMeasurementStatus::ADC_SATURATION:
            return "ADC_SATURATION";
        }
        return "NOT_READY";
    }
}
