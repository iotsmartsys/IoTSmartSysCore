#pragma once

#include <cstdint>
#include <optional>

#include "Contracts/Sensors/ICurrentSensor.h"

namespace iotsmartsys::platform::arduino
{
    class ACS712C30ACurrentSensor final : public iotsmartsys::core::ICurrentSensor
    {
    public:
        explicit ACS712C30ACurrentSensor(const iotsmartsys::core::CurrentSensorConfig &config);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;

        const iotsmartsys::core::CurrentMeasurement &currentMeasurement() const override;
        std::optional<float> calibratedZeroAdcMv() const override;
        void requestZeroCalibration() override;

    private:
        enum class Phase
        {
            WARMUP,
            RECALIBRATION_SETTLE,
            CALIBRATING,
            MEASURING,
            ZERO_FAILED
        };

        bool adcOpportunityEligible(std::uint32_t nowUs) const;
        std::uint32_t readAdcMilliVolts(int pin, std::uint32_t nowUs);
        bool supplySampleDue(std::uint32_t nowMs) const;
        bool sampleSupply(std::uint32_t nowMs, std::uint32_t nowUs);
        bool supplyAllowsCalibration() const;
        void beginCalibration(bool initial, std::uint32_t nowMs);
        void abortCalibrationForSupply();
        void resetAccumulator();
        void collectCalibrationSample(std::uint32_t nowMs, std::uint32_t nowUs);
        void completeCalibration(std::uint32_t nowMs);
        void collectMeasurementSample(std::uint32_t nowMs, std::uint32_t nowUs);
        void completeMeasurement(std::uint32_t nowMs);
        void qualifyCurrent(float filteredMilliVolts);

        iotsmartsys::core::CurrentSensorConfig _config;
        iotsmartsys::core::CurrentMeasurement _measurement;
        iotsmartsys::core::CurrentMeasurement _measurementBeforeCalibration;
        Phase _phase{Phase::WARMUP};

        std::optional<float> _calibratedZeroMv;
        bool _setupComplete{false};
        bool _hasValidZero{false};
        bool _calibrationIsInitial{true};
        bool _recalibrationRequested{false};

        std::uint32_t _setupStartedMs{0};
        std::uint32_t _phaseStartedMs{0};
        std::uint32_t _lastAdcReadUs{0};
        std::uint32_t _lastSupplySampleMs{0};
        std::uint32_t _lastMeasurementCompletedMs{0};
        unsigned long _lastStateReadMs{0};
        bool _hasAdcRead{false};
        bool _hasSupplySample{false};

        double _sampleSumMv{0.0};
        std::uint32_t _sampleCount{0};
        bool _sampleSaturated{false};
        bool _batchActive{false};

        bool _filterInitialized{false};
        float _filteredMv{0.0f};
    };
}
