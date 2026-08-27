#pragma once

#include <optional>
#include <string>

#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Sensors/ICurrentSensor.h"

namespace iotsmartsys::core
{
    class CurrentSensorCapability : public ICapability
    {
    public:
        CurrentSensorCapability(const std::string &capabilityName,
                                ICurrentSensor &sensor,
                                ICapabilityEventSink *eventSink);

        void setup() override;
        void handle() override;

        const CurrentMeasurement &currentMeasurement() const;
        std::optional<float> calibratedZeroAdcMv() const;
        void requestZeroCalibration();

    private:
        static std::string formatCurrentValue(const CurrentMeasurement &measurement);
        void publishIfChanged(const CurrentMeasurement &measurement);

        ICurrentSensor &_sensor;
        bool _published{false};
        std::uint64_t _lastEvaluationMs{0};
        std::string _lastMeasurementStatus;
        std::string _lastSupplyStatus;
    };
}
