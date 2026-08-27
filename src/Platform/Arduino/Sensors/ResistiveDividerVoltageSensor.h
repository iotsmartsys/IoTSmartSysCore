#pragma once

#include <cstdint>

#include "Contracts/Sensors/IVoltageSensor.h"

namespace iotsmartsys::platform::arduino
{
    class ResistiveDividerVoltageSensor final : public iotsmartsys::core::IVoltageSensor
    {
    public:
        explicit ResistiveDividerVoltageSensor(const iotsmartsys::core::VoltageSensorConfig &config);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;

        const iotsmartsys::core::VoltageMeasurement &voltageMeasurement() const override;

    private:
        static bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval);
        bool sampleEligible(std::uint32_t nowUs) const;
        void completeMeasurement(std::uint32_t nowMs);
        void resetAccumulator();

        iotsmartsys::core::VoltageSensorConfig _config;
        iotsmartsys::core::VoltageMeasurement _measurement;
        double _dividerRatio{0.0};
        double _sampleSumMv{0.0};
        std::uint32_t _sampleCount{0};
        std::uint32_t _lastSampleUs{0};
        std::uint32_t _lastMeasurementCompletedMs{0};
        unsigned long _lastStateReadMs{0};
        bool _setupComplete{false};
        bool _hasSample{false};
        bool _hasCompletedMeasurement{false};
        bool _batchActive{false};
    };
}
