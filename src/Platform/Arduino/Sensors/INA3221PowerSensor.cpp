#include "Platform/Arduino/Sensors/INA3221PowerSensor.h"

#include <Arduino.h>
#include <cmath>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    namespace
    {
        constexpr const char *kLogTag = "INA3221_POWER";
    }

    INA3221PowerSensor::INA3221PowerSensor(
        INA3221Device &device, const INA3221PowerSensorConfig &config)
        : _device(device), _config(config)
    {
    }

    void INA3221PowerSensor::setup()
    {
        _measurement = {};
        _lastEvaluationMs = 0;
        _lastStateReadMs = 0;
        _evaluated = false;
        _setupComplete = false;

        if (!configValid())
        {
            iotsmartsys::core::Log::get().error(kLogTag, "Invalid power sensor configuration.");
            return;
        }
        if (!setupDevice())
        {
            iotsmartsys::core::Log::get().error(kLogTag, "Shared device or shunt is unavailable.");
            return;
        }
        _setupComplete = true;
    }

    void INA3221PowerSensor::handle()
    {
        if (!_setupComplete)
            return;

        const std::uint32_t nowMs = nowMillis();
        if (_evaluated && static_cast<std::uint32_t>(nowMs - _lastEvaluationMs) < _config.readingIntervalMs)
            return;

        _evaluated = true;
        _lastEvaluationMs = nowMs;
        _lastStateReadMs = nowMs;

        if (!deviceAvailable())
        {
            invalidate(iotsmartsys::core::PowerMeasurementStatus::NOT_READY);
            return;
        }

        const float voltageV = readBusVoltage(_config.channel);
        const float currentA = _config.polarity * readCurrentAmps(_config.channel);
        if (!std::isfinite(voltageV) || !std::isfinite(currentA))
        {
            invalidate(iotsmartsys::core::PowerMeasurementStatus::NOT_READY);
            return;
        }

        const float magnitudeA = std::fabs(currentA);
        if (voltageV < _config.minimumVoltageV || voltageV >= _config.maximumVoltageV ||
            magnitudeA > _config.maximumAbsoluteCurrentA)
        {
            invalidate(iotsmartsys::core::PowerMeasurementStatus::INPUT_INVALID);
            return;
        }

        const double qualifiedCurrentA = magnitudeA < _config.deadbandA ? 0.0 : currentA;
        const double powerW = std::fabs(static_cast<double>(voltageV) * qualifiedCurrentA);
        if (!std::isfinite(powerW))
        {
            invalidate(iotsmartsys::core::PowerMeasurementStatus::INPUT_INVALID);
            return;
        }

        _measurement.powerW = powerW;
        _measurement.measurementStatus = iotsmartsys::core::PowerMeasurementStatus::ESTIMATED;
    }

    long INA3221PowerSensor::lastStateReadMillis() const
    {
        return static_cast<long>(_lastStateReadMs);
    }

    const iotsmartsys::core::PowerMeasurement &INA3221PowerSensor::powerMeasurement() const
    {
        return _measurement;
    }

    bool INA3221PowerSensor::setupDevice()
    {
        return _device.setup() &&
               _device.configureShuntResistance(_config.channel, _config.shuntResistanceOhms);
    }

    bool INA3221PowerSensor::deviceAvailable() const { return _device.available(); }
    float INA3221PowerSensor::readBusVoltage(std::uint8_t channel) { return _device.busVoltage(channel); }
    float INA3221PowerSensor::readCurrentAmps(std::uint8_t channel) { return _device.currentAmps(channel); }
    std::uint32_t INA3221PowerSensor::nowMillis() const { return millis(); }

    bool INA3221PowerSensor::configValid() const
    {
        if (_config.channel > 2 || _config.readingIntervalMs == 0 ||
            !std::isfinite(_config.shuntResistanceOhms) || _config.shuntResistanceOhms <= 0.0f ||
            (_config.polarity != 1.0f && _config.polarity != -1.0f) ||
            !std::isfinite(_config.deadbandA) || _config.deadbandA < 0.0f ||
            !std::isfinite(_config.minimumReportableA) || _config.minimumReportableA < _config.deadbandA ||
            !std::isfinite(_config.maximumAbsoluteCurrentA) || _config.maximumAbsoluteCurrentA <= 0.0f ||
            _config.minimumReportableA > _config.maximumAbsoluteCurrentA ||
            !std::isfinite(_config.minimumVoltageV) || _config.minimumVoltageV < 0.0f ||
            !std::isfinite(_config.maximumVoltageV) ||
            _config.maximumVoltageV <= _config.minimumVoltageV || _config.maximumVoltageV > 26.0f)
            return false;

        const float maximumMeasurableCurrentA = 0.1638f / _config.shuntResistanceOhms;
        return std::isfinite(maximumMeasurableCurrentA) &&
               _config.maximumAbsoluteCurrentA <= maximumMeasurableCurrentA;
    }

    void INA3221PowerSensor::invalidate(iotsmartsys::core::PowerMeasurementStatus status)
    {
        _measurement.powerW.reset();
        _measurement.measurementStatus = status;
    }
}
