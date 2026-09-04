#pragma once

#include <cstdint>
#include <string>

#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Capabilities/PowerEnergyTypes.h"
#include "Contracts/Sensors/IPowerSensor.h"

namespace iotsmartsys::core
{
    class PowerEnergyCapability : public ICapability
    {
    public:
        PowerEnergyCapability(const std::string &capabilityName,
                              IPowerSensor &powerSensor,
                              ICapabilityEventSink *eventSink,
                              std::uint32_t readingIntervalMs = 1000);

        void setup() override;
        void handle() override;

        const PowerEnergyMeasurement &powerEnergyMeasurement() const;
        void resetEnergy();

    private:
        static PowerEnergyMeasurementStatus classify(const PowerMeasurement &power);
        static std::string formatFixed(double value, unsigned precision);

        void evaluate(std::uint32_t nowMs, const PowerMeasurement &power);
        void invalidate(PowerEnergyMeasurementStatus status);
        void publishIfChanged();

        IPowerSensor &_powerSensor;
        std::uint32_t _readingIntervalMs{1000};
        PowerEnergyMeasurement _measurement;

        bool _evaluated{false};
        bool _published{false};
        bool _hasIntegrationBaseline{false};
        std::uint32_t _lastEvaluationMs{0};
        std::uint32_t _integrationBaselineMs{0};
        double _previousPowerW{0.0};
        std::string _lastMeasurementStatus;
        std::string _lastEnergyWh;
    };
}
