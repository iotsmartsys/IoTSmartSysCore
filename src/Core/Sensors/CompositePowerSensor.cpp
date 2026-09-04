#include "Core/Sensors/CompositePowerSensor.h"

#include <Arduino.h>
#include <cmath>

namespace iotsmartsys::core
{
    CompositePowerSensor::CompositePowerSensor(IVoltageSensor &voltageSensor,
                                               ICurrentSensor &currentSensor,
                                               std::uint32_t readingIntervalMs)
        : _voltageSensor(voltageSensor),
          _currentSensor(currentSensor),
          _readingIntervalMs(readingIntervalMs)
    {
    }

    void CompositePowerSensor::setup()
    {
        _measurement = {};
        _lastEvaluationMs = 0;
        _lastStateReadMs = 0;
        _evaluated = false;
    }

    void CompositePowerSensor::handle()
    {
        if (_readingIntervalMs == 0)
        {
            invalidate(PowerMeasurementStatus::INPUT_INVALID);
            return;
        }

        const std::uint32_t nowMs = millis();
        if (_evaluated && static_cast<std::uint32_t>(nowMs - _lastEvaluationMs) < _readingIntervalMs)
            return;

        _evaluated = true;
        _lastEvaluationMs = nowMs;
        _lastStateReadMs = nowMs;

        const VoltageMeasurement &voltage = _voltageSensor.voltageMeasurement();
        const CurrentMeasurement &current = _currentSensor.currentMeasurement();
        const PowerMeasurementStatus status = classify(voltage, current);
        if (status == PowerMeasurementStatus::NOT_READY ||
            status == PowerMeasurementStatus::INPUT_INVALID)
        {
            invalidate(status);
            return;
        }

        const double powerW = std::fabs(static_cast<double>(*voltage.voltageV) *
                                        static_cast<double>(*current.currentA));
        if (!std::isfinite(powerW))
        {
            invalidate(PowerMeasurementStatus::INPUT_INVALID);
            return;
        }
        _measurement.powerW = powerW;
        _measurement.measurementStatus = status;
    }

    long CompositePowerSensor::lastStateReadMillis() const
    {
        return static_cast<long>(_lastStateReadMs);
    }

    const PowerMeasurement &CompositePowerSensor::powerMeasurement() const
    {
        return _measurement;
    }

    PowerMeasurementStatus CompositePowerSensor::classify(
        const VoltageMeasurement &voltage, const CurrentMeasurement &current)
    {
        const bool invalid =
            voltage.measurementStatus == VoltageMeasurementStatus::BELOW_MINIMUM ||
            voltage.measurementStatus == VoltageMeasurementStatus::ADC_SATURATION ||
            current.measurementStatus == CurrentMeasurementStatus::ZERO_CALIBRATION_FAILED ||
            current.measurementStatus == CurrentMeasurementStatus::OUT_OF_CALIBRATED_RANGE ||
            current.measurementStatus == CurrentMeasurementStatus::OVERCURRENT_OR_SATURATION ||
            current.supplyStatus == CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE;
        if (invalid)
            return PowerMeasurementStatus::INPUT_INVALID;

        if (voltage.measurementStatus == VoltageMeasurementStatus::NOT_READY ||
            current.measurementStatus == CurrentMeasurementStatus::NOT_READY ||
            current.measurementStatus == CurrentMeasurementStatus::CALIBRATING ||
            current.supplyStatus == CurrentSupplyStatus::UNKNOWN)
            return PowerMeasurementStatus::NOT_READY;

        if (voltage.measurementStatus != VoltageMeasurementStatus::VALID ||
            (current.measurementStatus != CurrentMeasurementStatus::VALID &&
             current.measurementStatus != CurrentMeasurementStatus::ESTIMATED) ||
            !voltage.voltageV || !current.currentA ||
            !std::isfinite(*voltage.voltageV) || !std::isfinite(*current.currentA))
            return PowerMeasurementStatus::INPUT_INVALID;

        if (current.measurementStatus == CurrentMeasurementStatus::ESTIMATED ||
            current.supplyStatus == CurrentSupplyStatus::NOT_MONITORED)
            return PowerMeasurementStatus::ESTIMATED;

        return current.supplyStatus == CurrentSupplyStatus::IN_RANGE
                   ? PowerMeasurementStatus::VALID
                   : PowerMeasurementStatus::INPUT_INVALID;
    }

    void CompositePowerSensor::invalidate(PowerMeasurementStatus status)
    {
        _measurement.powerW.reset();
        _measurement.measurementStatus = status;
    }
}
