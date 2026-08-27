#pragma once

#include <cstdint>
#include <string>

#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Sensors/IVoltageSensor.h"

namespace iotsmartsys::core
{
    class VoltageSensorCapability : public ICapability
    {
    public:
        VoltageSensorCapability(const std::string &capabilityName,
                                IVoltageSensor &sensor,
                                ICapabilityEventSink *eventSink);
        VoltageSensorCapability(const std::string &capabilityName,
                                IVoltageSensor &sensor,
                                ICapabilityEventSink *eventSink,
                                std::uint32_t evaluationIntervalMs);

        void setup() override;
        void handle() override;

        const VoltageMeasurement &voltageMeasurement() const;

    private:
        static std::string formatVoltageValue(const VoltageMeasurement &measurement);
        void publishIfChanged(const VoltageMeasurement &measurement);

        IVoltageSensor &_sensor;
        std::uint32_t _evaluationIntervalMs{1000};
        bool _published{false};
        std::uint64_t _lastEvaluationMs{0};
        std::string _lastMeasurementStatus;
    };
}
