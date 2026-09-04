#include "Contracts/Capabilities/PowerEnergyCapability.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace iotsmartsys::core
{
    PowerEnergyCapability::PowerEnergyCapability(const std::string &capabilityName,
                                                 IPowerSensor &powerSensor,
                                                 ICapabilityEventSink *eventSink,
                                                 std::uint32_t readingIntervalMs)
        : ICapability(eventSink, capabilityName.c_str(), POWER_ENERGY_TYPE, ""),
          _powerSensor(powerSensor),
          _readingIntervalMs(readingIntervalMs)
    {
    }

    void PowerEnergyCapability::setup()
    {
        _powerSensor.setup();
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
        _powerSensor.handle();
        const std::uint32_t nowMs = static_cast<std::uint32_t>(timeProvider.nowMs());
        if (_evaluated && static_cast<std::uint32_t>(nowMs - _lastEvaluationMs) < _readingIntervalMs)
        {
            return;
        }

        _evaluated = true;
        _lastEvaluationMs = nowMs;

        const PowerMeasurement &power = _powerSensor.powerMeasurement();
        evaluate(nowMs, power);
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

    PowerEnergyMeasurementStatus PowerEnergyCapability::classify(const PowerMeasurement &power)
    {
        const bool requiresValue = power.measurementStatus == PowerMeasurementStatus::VALID ||
                                   power.measurementStatus == PowerMeasurementStatus::ESTIMATED;
        const bool coherentValue = power.powerW && std::isfinite(*power.powerW) && *power.powerW >= 0.0;
        if ((requiresValue && !coherentValue) || (!requiresValue && power.powerW))
            return PowerEnergyMeasurementStatus::INPUT_INVALID;

        switch (power.measurementStatus)
        {
        case PowerMeasurementStatus::NOT_READY:
            return PowerEnergyMeasurementStatus::NOT_READY;
        case PowerMeasurementStatus::VALID:
            return PowerEnergyMeasurementStatus::VALID;
        case PowerMeasurementStatus::ESTIMATED:
            return PowerEnergyMeasurementStatus::ESTIMATED;
        case PowerMeasurementStatus::INPUT_INVALID:
            return PowerEnergyMeasurementStatus::INPUT_INVALID;
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

    void PowerEnergyCapability::evaluate(std::uint32_t nowMs, const PowerMeasurement &power)
    {
        const PowerEnergyMeasurementStatus status = classify(power);
        if (status == PowerEnergyMeasurementStatus::NOT_READY ||
            status == PowerEnergyMeasurementStatus::INPUT_INVALID)
        {
            invalidate(status);
            return;
        }

        const double powerW = *power.powerW;
        if (!std::isfinite(powerW))
        {
            invalidate(PowerEnergyMeasurementStatus::INPUT_INVALID);
            return;
        }

        double nextEnergyWh = _measurement.energyWh;
        if (_hasIntegrationBaseline)
        {
            const std::uint32_t elapsedMs = static_cast<std::uint32_t>(nowMs - _integrationBaselineMs);
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
