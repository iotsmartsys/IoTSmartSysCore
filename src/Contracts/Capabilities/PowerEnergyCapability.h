#pragma once

#include <cstdint>
#include <string>

#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Capabilities/PowerEnergyTypes.h"
#include "Contracts/Sensors/ICurrentSensor.h"
#include "Contracts/Sensors/IVoltageSensor.h"

namespace iotsmartsys::core
{
    class PowerEnergyCapability : public ICapability
    {
    public:
        PowerEnergyCapability(const std::string &capabilityName,
                              IVoltageSensor &voltageSensor,
                              ICurrentSensor &currentSensor,
                              ICapabilityEventSink *eventSink,
                              std::uint32_t readingIntervalMs = 1000);

        void setup() override;
        void handle() override;

        const PowerEnergyMeasurement &powerEnergyMeasurement() const;
        void resetEnergy();

    private:
        static PowerEnergyMeasurementStatus classifyInputs(
            const VoltageMeasurement &voltage,
            const CurrentMeasurement &current);
        static std::string formatFixed(double value, unsigned precision);

        void evaluate(std::uint64_t nowMs,
                      const VoltageMeasurement &voltage,
                      const CurrentMeasurement &current);
        void invalidate(PowerEnergyMeasurementStatus status);
        void publishIfChanged();

        IVoltageSensor &_voltageSensor;
        ICurrentSensor &_currentSensor;
        std::uint32_t _readingIntervalMs{1000};
        PowerEnergyMeasurement _measurement;

        bool _evaluated{false};
        bool _published{false};
        bool _hasIntegrationBaseline{false};
        std::uint64_t _lastEvaluationMs{0};
        std::uint64_t _integrationBaselineMs{0};
        double _previousPowerW{0.0};
        std::string _lastMeasurementStatus;
        std::string _lastEnergyWh;
    };
}
