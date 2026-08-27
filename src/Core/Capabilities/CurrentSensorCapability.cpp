#include "Contracts/Capabilities/CurrentSensorCapability.h"

#include <cmath>
#include <cstdio>

namespace iotsmartsys::core
{
    CurrentSensorCapability::CurrentSensorCapability(const std::string &capabilityName,
                                                     ICurrentSensor &sensor,
                                                     ICapabilityEventSink *eventSink)
        : CurrentSensorCapability(capabilityName, sensor, eventSink, 1000)
    {
    }

    CurrentSensorCapability::CurrentSensorCapability(const std::string &capabilityName,
                                                     ICurrentSensor &sensor,
                                                     ICapabilityEventSink *eventSink,
                                                     std::uint32_t evaluationIntervalMs)
        : ICapability(eventSink, capabilityName.c_str(), CURRENT_SENSOR_TYPE, ""),
          _sensor(sensor),
          _evaluationIntervalMs(evaluationIntervalMs)
    {
    }

    void CurrentSensorCapability::setup()
    {
        ICapability::setup();
        _sensor.setup();
        _published = false;
        _lastEvaluationMs = 0;
    }

    void CurrentSensorCapability::handle()
    {
        _sensor.handle();

        const std::uint64_t now = timeProvider.nowMs();
        if (_published && now - _lastEvaluationMs < _evaluationIntervalMs)
        {
            return;
        }

        _lastEvaluationMs = now;
        publishIfChanged(_sensor.currentMeasurement());
    }

    const CurrentMeasurement &CurrentSensorCapability::currentMeasurement() const
    {
        return _sensor.currentMeasurement();
    }

    std::optional<float> CurrentSensorCapability::calibratedZeroAdcMv() const
    {
        return _sensor.calibratedZeroAdcMv();
    }

    void CurrentSensorCapability::requestZeroCalibration()
    {
        _sensor.requestZeroCalibration();
    }

    std::string CurrentSensorCapability::formatCurrentValue(const CurrentMeasurement &measurement)
    {
        if (!measurement.currentA ||
            measurement.supplyStatus == CurrentSupplyStatus::UNKNOWN ||
            measurement.supplyStatus == CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE ||
            (measurement.measurementStatus != CurrentMeasurementStatus::ESTIMATED &&
             measurement.measurementStatus != CurrentMeasurementStatus::VALID))
        {
            return "";
        }

        long milliAmps = std::lround(static_cast<double>(*measurement.currentA) * 1000.0);
        if (milliAmps == 0)
        {
            return "0.000";
        }

        const bool negative = milliAmps < 0;
        const unsigned long magnitude = static_cast<unsigned long>(negative ? -milliAmps : milliAmps);
        char buffer[24];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%s%lu.%03lu",
                      negative ? "-" : "",
                      magnitude / 1000UL,
                      magnitude % 1000UL);
        return buffer;
    }

    void CurrentSensorCapability::publishIfChanged(const CurrentMeasurement &measurement)
    {
        const std::string nextValue = formatCurrentValue(measurement);
        const std::string nextMeasurementStatus = toString(measurement.measurementStatus);
        const std::string nextSupplyStatus = toString(measurement.supplyStatus);

        if (_published && value == nextValue &&
            _lastMeasurementStatus == nextMeasurementStatus &&
            _lastSupplyStatus == nextSupplyStatus)
        {
            return;
        }

        value = nextValue;
        _lastMeasurementStatus = nextMeasurementStatus;
        _lastSupplyStatus = nextSupplyStatus;
        _published = true;

        if (event_sink)
        {
            CapabilityStateChanged event(capability_name.c_str(), value.c_str(), type.c_str());
            event.measurementStatus = _lastMeasurementStatus;
            event.supplyStatus = _lastSupplyStatus;
            event_sink->onStateChanged(event);
        }
    }
}
