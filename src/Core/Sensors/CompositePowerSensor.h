#pragma once

#include <cstdint>

#include "Contracts/Sensors/ICurrentSensor.h"
#include "Contracts/Sensors/IPowerSensor.h"
#include "Contracts/Sensors/IVoltageSensor.h"

namespace iotsmartsys::core
{
    class CompositePowerSensor final : public IPowerSensor
    {
    public:
        CompositePowerSensor(IVoltageSensor &voltageSensor,
                             ICurrentSensor &currentSensor,
                             std::uint32_t readingIntervalMs = 1000);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;
        const PowerMeasurement &powerMeasurement() const override;

    private:
        static PowerMeasurementStatus classify(const VoltageMeasurement &voltage,
                                               const CurrentMeasurement &current);
        void invalidate(PowerMeasurementStatus status);

        IVoltageSensor &_voltageSensor;
        ICurrentSensor &_currentSensor;
        std::uint32_t _readingIntervalMs;
        PowerMeasurement _measurement;
        std::uint32_t _lastEvaluationMs{0};
        unsigned long _lastStateReadMs{0};
        bool _evaluated{false};
    };
}
