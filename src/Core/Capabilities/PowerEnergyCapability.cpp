#include "Contracts/Capabilities/PowerEnergyCapability.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace iotsmartsys::core
{
    PowerEnergyCapability::PowerEnergyCapability(const std::string &capabilityName,
                                                 IVoltageSensor &voltageSensor,
                                                 ICurrentSensor &currentSensor,
                                                 ICapabilityEventSink *eventSink,
                                                 std::uint32_t readingIntervalMs)
        : ICapability(eventSink, capabilityName.c_str(), POWER_ENERGY_TYPE, ""),
          _voltageSensor(voltageSensor),
          _currentSensor(currentSensor),
          _readingIntervalMs(readingIntervalMs)
    {
    }

    void PowerEnergyCapability::setup()
    {
        ICapability::setup();
        value.clear();
        _measurement.powerW.reset();
        _measurement.energyWh = 0.0;
        _measurement.measurementStatus = PowerEnergyMeasurementStatus::NOT_READY;
        _evaluated = false;
        _published = false;
        _hasIntegrationBaseline = false;
        _lastEvaluationMs = 0;
        _integrationBaselineMs = 0;
        _previousPowerW = 0.0;
        _lastMeasurementStatus.clear();
        _lastEnergyWh.clear();
    }

    void PowerEnergyCapability::handle()
    {
        const std::uint64_t nowMs = timeProvider.nowMs();
        if (_evaluated && nowMs - _lastEvaluationMs < _readingIntervalMs)
        {
            return;
        }

        _evaluated = true;
        _lastEvaluationMs = nowMs;

        const VoltageMeasurement &voltage = _voltageSensor.voltageMeasurement();
        const CurrentMeasurement &current = _currentSensor.currentMeasurement();
        evaluate(nowMs, voltage, current);
        publishIfChanged();
    }

    const PowerEnergyMeasurement &PowerEnergyCapability::powerEnergyMeasurement() const
    {
        return _measurement;
    }

    void PowerEnergyCapability::resetEnergy()
    {
        _measurement.energyWh = 0.0;
        _hasIntegrationBaseline = false;
        _integrationBaselineMs = 0;
        _previousPowerW = 0.0;
    }

    PowerEnergyMeasurementStatus PowerEnergyCapability::classifyInputs(
        const VoltageMeasurement &voltage,
        const CurrentMeasurement &current)
    {
        const bool voltageInvalid =
            voltage.measurementStatus == VoltageMeasurementStatus::BELOW_MINIMUM ||
            voltage.measurementStatus == VoltageMeasurementStatus::ADC_SATURATION;
        const bool currentInvalid =
            current.measurementStatus == CurrentMeasurementStatus::ZERO_CALIBRATION_FAILED ||
            current.measurementStatus == CurrentMeasurementStatus::OUT_OF_CALIBRATED_RANGE ||
            current.measurementStatus == CurrentMeasurementStatus::OVERCURRENT_OR_SATURATION ||
            current.supplyStatus == CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE;

        if (voltageInvalid || currentInvalid)
        {
            return PowerEnergyMeasurementStatus::INPUT_INVALID;
        }

        if (voltage.measurementStatus == VoltageMeasurementStatus::NOT_READY ||
            current.measurementStatus == CurrentMeasurementStatus::NOT_READY ||
            current.measurementStatus == CurrentMeasurementStatus::CALIBRATING ||
            current.supplyStatus == CurrentSupplyStatus::UNKNOWN)
        {
            return PowerEnergyMeasurementStatus::NOT_READY;
        }

        if (voltage.measurementStatus != VoltageMeasurementStatus::VALID ||
            (current.measurementStatus != CurrentMeasurementStatus::VALID &&
             current.measurementStatus != CurrentMeasurementStatus::ESTIMATED) ||
            !voltage.voltageV || !current.currentA ||
            !std::isfinite(*voltage.voltageV) || !std::isfinite(*current.currentA))
        {
            return PowerEnergyMeasurementStatus::INPUT_INVALID;
        }

        if (current.measurementStatus == CurrentMeasurementStatus::ESTIMATED ||
            current.supplyStatus == CurrentSupplyStatus::NOT_MONITORED)
        {
            return PowerEnergyMeasurementStatus::ESTIMATED;
        }

        if (current.measurementStatus == CurrentMeasurementStatus::VALID &&
            current.supplyStatus == CurrentSupplyStatus::IN_RANGE)
        {
            return PowerEnergyMeasurementStatus::VALID;
        }

        return PowerEnergyMeasurementStatus::INPUT_INVALID;
    }

    std::string PowerEnergyCapability::formatFixed(double number, unsigned precision)
    {
        if (!std::isfinite(number) || number < 0.0 || precision > 3)
        {
            return "";
        }
        if (number == 0.0)
        {
            return precision == 2 ? "0.00" : "0.000";
        }

        char buffer[384];
        const char *format = precision == 2 ? "%.2f" : "%.3f";
        const int length = std::snprintf(buffer, sizeof(buffer), format, number);
        if (length < 0 || static_cast<std::size_t>(length) >= sizeof(buffer))
        {
            return "";
        }

        std::string result(buffer, static_cast<std::size_t>(length));
        std::replace(result.begin(), result.end(), ',', '.');
        return result;
    }

    void PowerEnergyCapability::evaluate(std::uint64_t nowMs,
                                         const VoltageMeasurement &voltage,
                                         const CurrentMeasurement &current)
    {
        const PowerEnergyMeasurementStatus status = classifyInputs(voltage, current);
        if (status == PowerEnergyMeasurementStatus::NOT_READY ||
            status == PowerEnergyMeasurementStatus::INPUT_INVALID)
        {
            invalidate(status);
            return;
        }

        const double powerW = std::fabs(static_cast<double>(*voltage.voltageV) *
                                        static_cast<double>(*current.currentA));
        if (!std::isfinite(powerW))
        {
            invalidate(PowerEnergyMeasurementStatus::INPUT_INVALID);
            return;
        }

        double nextEnergyWh = _measurement.energyWh;
        if (_hasIntegrationBaseline)
        {
            const std::uint64_t elapsedMs = nowMs - _integrationBaselineMs;
            const double deltaEnergyWh = ((_previousPowerW + powerW) / 2.0) *
                                         static_cast<double>(elapsedMs) / 3600000.0;
            const double candidateEnergyWh = nextEnergyWh + deltaEnergyWh;
            if (!std::isfinite(deltaEnergyWh) || deltaEnergyWh < 0.0 ||
                !std::isfinite(candidateEnergyWh) || candidateEnergyWh < nextEnergyWh)
            {
                invalidate(PowerEnergyMeasurementStatus::INPUT_INVALID);
                return;
            }
            nextEnergyWh = candidateEnergyWh;
        }

        _measurement.powerW = powerW;
        _measurement.energyWh = nextEnergyWh;
        _measurement.measurementStatus = status;
        _previousPowerW = powerW;
        _integrationBaselineMs = nowMs;
        _hasIntegrationBaseline = true;
    }

    void PowerEnergyCapability::invalidate(PowerEnergyMeasurementStatus status)
    {
        _measurement.powerW.reset();
        _measurement.measurementStatus = status;
        _hasIntegrationBaseline = false;
        _integrationBaselineMs = 0;
        _previousPowerW = 0.0;
    }

    void PowerEnergyCapability::publishIfChanged()
    {
        const std::string nextValue = _measurement.powerW
                                          ? formatFixed(*_measurement.powerW, 2)
                                          : "";
        const std::string nextMeasurementStatus = toString(_measurement.measurementStatus);
        const std::string nextEnergyWh = formatFixed(_measurement.energyWh, 3);

        if (_published && value == nextValue &&
            _lastMeasurementStatus == nextMeasurementStatus &&
            _lastEnergyWh == nextEnergyWh)
        {
            return;
        }

        value = nextValue;
        _lastMeasurementStatus = nextMeasurementStatus;
        _lastEnergyWh = nextEnergyWh;
        _published = true;

        if (event_sink)
        {
            CapabilityStateChanged event(capability_name.c_str(), value.c_str(), type.c_str());
            event.measurementStatus = _lastMeasurementStatus;
            event.energyWh = _lastEnergyWh;
            event_sink->onStateChanged(event);
        }
    }
}
