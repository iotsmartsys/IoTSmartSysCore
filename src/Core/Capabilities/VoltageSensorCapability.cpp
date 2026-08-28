#include "Contracts/Capabilities/VoltageSensorCapability.h"

#include <cmath>
#include <cstdio>

namespace iotsmartsys::core
{
    VoltageSensorCapability::VoltageSensorCapability(const std::string &capabilityName,
                                                     IVoltageSensor &sensor,
                                                     ICapabilityEventSink *eventSink)
        : VoltageSensorCapability(capabilityName, sensor, eventSink, 1000)
    {
    }

    VoltageSensorCapability::VoltageSensorCapability(const std::string &capabilityName,
                                                     IVoltageSensor &sensor,
                                                     ICapabilityEventSink *eventSink,
                                                     std::uint32_t evaluationIntervalMs)
        : ICapability(eventSink, capabilityName.c_str(), VOLTAGE_SENSOR_TYPE, ""),
          _sensor(sensor),
          _evaluationIntervalMs(evaluationIntervalMs)
    {
    }

    void VoltageSensorCapability::setup()
    {
        ICapability::setup();
        _sensor.setup();
        _published = false;
        _lastEvaluationMs = 0;
    }

    void VoltageSensorCapability::handle()
    {
        _sensor.handle();

        const std::uint64_t now = timeProvider.nowMs();
        if (_published && now - _lastEvaluationMs < _evaluationIntervalMs)
        {
            return;
        }

        _lastEvaluationMs = now;
        publishIfChanged(_sensor.voltageMeasurement());
    }

    const VoltageMeasurement &VoltageSensorCapability::voltageMeasurement() const
    {
        return _sensor.voltageMeasurement();
    }

    std::string VoltageSensorCapability::formatVoltageValue(const VoltageMeasurement &measurement)
    {
        if (measurement.measurementStatus == VoltageMeasurementStatus::BELOW_MINIMUM)
        {
            return "-1000.00";
        }
        if (measurement.measurementStatus != VoltageMeasurementStatus::VALID || !measurement.voltageV)
        {
            return "";
        }

        long centiVolts = std::lround(static_cast<double>(*measurement.voltageV) * 100.0);
        if (centiVolts == 0)
        {
            return "0.00";
        }

        const bool negative = centiVolts < 0;
        const unsigned long magnitude = static_cast<unsigned long>(negative ? -centiVolts : centiVolts);
        char buffer[24];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%s%lu.%02lu",
                      negative ? "-" : "",
                      magnitude / 100UL,
                      magnitude % 100UL);
        return buffer;
    }

    void VoltageSensorCapability::publishIfChanged(const VoltageMeasurement &measurement)
    {
        const std::string nextValue = formatVoltageValue(measurement);
        const std::string nextMeasurementStatus = toString(measurement.measurementStatus);

        if (_published && value == nextValue && _lastMeasurementStatus == nextMeasurementStatus)
        {
            return;
        }

        value = nextValue;
        _lastMeasurementStatus = nextMeasurementStatus;
        _published = true;

        if (event_sink)
        {
            CapabilityStateChanged event(capability_name.c_str(), value.c_str(), type.c_str());
            event.measurementStatus = _lastMeasurementStatus;
            event_sink->onStateChanged(event);
        }
    }
}
