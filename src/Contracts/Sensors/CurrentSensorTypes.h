#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace iotsmartsys::core
{
    enum class CurrentMeasurementStatus
    {
        NOT_READY,
        CALIBRATING,
        ZERO_CALIBRATION_FAILED,
        ESTIMATED,
        VALID,
        OUT_OF_CALIBRATED_RANGE,
        OVERCURRENT_OR_SATURATION
    };

    enum class CurrentSupplyStatus
    {
        UNKNOWN,
        IN_RANGE,
        SUPPLY_OUT_OF_RANGE,
        NOT_MONITORED
    };

    enum class CurrentSensorQualification
    {
        MANUFACTURER_SUPPORTED,
        PROJECT_VALIDATED
    };

    enum class CurrentSensorAdcAttenuation
    {
        FULL_RANGE
    };

    enum class CurrentZeroCalibrationMode
    {
        STARTUP_AND_ON_REQUEST
    };

    struct CurrentMeasurement
    {
        std::optional<float> currentA;
        CurrentMeasurementStatus measurementStatus{CurrentMeasurementStatus::NOT_READY};
        CurrentSupplyStatus supplyStatus{CurrentSupplyStatus::NOT_MONITORED};
    };

    struct CurrentSensorConfig
    {
        std::string id;
        int adcPin{-1};
        std::uint8_t adcResolutionBits{12};
        CurrentSensorAdcAttenuation adcAttenuation{CurrentSensorAdcAttenuation::FULL_RANGE};
        float adcMinimumMv{150.0f};
        float adcMaximumMv{3100.0f};

        float supplyNominalMv{0.0f};
        float supplyValidMinimumMv{0.0f};
        float supplyValidMaximumMv{0.0f};
        CurrentSensorQualification qualification{CurrentSensorQualification::PROJECT_VALIDATED};

        float outputToAdcRatio{1.0f};
        float nominalZeroAdcMv{0.0f};
        float sensitivityAdcMvPerA{0.0f};
        float polarity{1.0f};

        CurrentZeroCalibrationMode zeroCalibrationMode{CurrentZeroCalibrationMode::STARTUP_AND_ON_REQUEST};
        std::uint32_t startupWarmupMs{60000};
        std::uint32_t recalibrationSettleMs{2000};
        std::uint32_t zeroCalibrationSamples{2000};
        float maximumZeroDeviationMv{100.0f};

        float calibratedMinimumA{0.50f};
        float calibratedMaximumA{15.00f};
        float physicalMinimumA{-30.00f};
        float physicalMaximumA{30.00f};

        float deadbandA{0.05f};
        float minimumReportableA{0.05f};
        std::uint32_t samplesPerReading{500};
        std::uint32_t sampleIntervalUs{1000};
        float lowPassAlpha{1.0f};
        std::uint32_t readingIntervalMs{500};

        float maximumAbsoluteErrorA{0.10f};
        float maximumRelativeErrorPercent{5.0f};

        int supplyMonitorAdcPin{-1};
        float supplyMonitorToVccRatio{1.0f};

        std::uint32_t capabilityEvaluationIntervalMs{1000};

        static CurrentSensorConfig ACS712_30A_5V(const std::string &id, int adcPin)
        {
            CurrentSensorConfig config;
            config.id = id;
            config.adcPin = adcPin;
            config.supplyNominalMv = 5000.0f;
            config.supplyValidMinimumMv = 4900.0f;
            config.supplyValidMaximumMv = 5100.0f;
            config.qualification = CurrentSensorQualification::MANUFACTURER_SUPPORTED;
            config.outputToAdcRatio = 0.666667f;
            config.nominalZeroAdcMv = 1666.7f;
            config.sensitivityAdcMvPerA = 43.05f;
            return config;
        }

        static CurrentSensorConfig ACS712_30A_3V3(const std::string &id, int adcPin)
        {
            CurrentSensorConfig config;
            config.id = id;
            config.adcPin = adcPin;
            config.supplyNominalMv = 3300.0f;
            config.supplyValidMinimumMv = 3200.0f;
            config.supplyValidMaximumMv = 3400.0f;
            config.qualification = CurrentSensorQualification::PROJECT_VALIDATED;
            config.outputToAdcRatio = 1.0f;
            config.nominalZeroAdcMv = 1650.0f;
            config.sensitivityAdcMvPerA = 43.56f;
            return config;
        }
    };

    inline const char *toString(CurrentMeasurementStatus status)
    {
        switch (status)
        {
        case CurrentMeasurementStatus::NOT_READY:
            return "NOT_READY";
        case CurrentMeasurementStatus::CALIBRATING:
            return "CALIBRATING";
        case CurrentMeasurementStatus::ZERO_CALIBRATION_FAILED:
            return "ZERO_CALIBRATION_FAILED";
        case CurrentMeasurementStatus::ESTIMATED:
            return "ESTIMATED";
        case CurrentMeasurementStatus::VALID:
            return "VALID";
        case CurrentMeasurementStatus::OUT_OF_CALIBRATED_RANGE:
            return "OUT_OF_CALIBRATED_RANGE";
        case CurrentMeasurementStatus::OVERCURRENT_OR_SATURATION:
            return "OVERCURRENT_OR_SATURATION";
        }
        return "NOT_READY";
    }

    inline const char *toString(CurrentSupplyStatus status)
    {
        switch (status)
        {
        case CurrentSupplyStatus::UNKNOWN:
            return "UNKNOWN";
        case CurrentSupplyStatus::IN_RANGE:
            return "IN_RANGE";
        case CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE:
            return "SUPPLY_OUT_OF_RANGE";
        case CurrentSupplyStatus::NOT_MONITORED:
            return "NOT_MONITORED";
        }
        return "UNKNOWN";
    }
}
