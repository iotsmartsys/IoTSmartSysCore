#include "Platform/Arduino/Sensors/INA3221CurrentSensor.h"

#include <Arduino.h>
#include <cmath>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    namespace
    {
        constexpr const char *kLogTag = "INA3221_CURRENT";

        bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval)
        {
            return static_cast<std::uint32_t>(now - since) >= interval;
        }
    }

    INA3221CurrentSensor::INA3221CurrentSensor(
        INA3221Device &device, const INA3221CurrentSensorConfig &config)
        : _device(device), _config(config)
    {
        _measurement.supplyStatus = iotsmartsys::core::CurrentSupplyStatus::NOT_MONITORED;
    }

    void INA3221CurrentSensor::setup()
    {
        _measurement.currentA.reset();
        _measurement.measurementStatus = iotsmartsys::core::CurrentMeasurementStatus::NOT_READY;
        _measurement.supplyStatus = iotsmartsys::core::CurrentSupplyStatus::NOT_MONITORED;
        _lastStateReadMs = 0;
        _lastMeasurementCompletedMs = 0;
        _hasCompletedMeasurement = false;
        _setupComplete = false;

        if (!configValid())
        {
            iotsmartsys::core::Log::get().error(kLogTag, "Invalid current sensor configuration.");
            return;
        }
        if (!_device.setup() ||
            !_device.configureShuntResistance(_config.channel, _config.shuntResistanceOhms))
        {
            iotsmartsys::core::Log::get().error(kLogTag, "Shared device or shunt is unavailable.");
            return;
        }
        _setupComplete = true;
    }

    void INA3221CurrentSensor::handle()
    {
        if (!_setupComplete || !_device.available())
        {
            return;
        }

        const std::uint32_t nowMs = millis();
        if (_hasCompletedMeasurement &&
            !elapsed(nowMs, _lastMeasurementCompletedMs, _config.readingIntervalMs))
        {
            return;
        }

        const float currentA = _config.polarity * _device.currentAmps(_config.channel);
        if (!std::isfinite(currentA))
        {
            _measurement.currentA.reset();
            _measurement.measurementStatus = iotsmartsys::core::CurrentMeasurementStatus::NOT_READY;
            return;
        }

        const float magnitudeA = std::fabs(currentA);
        if (magnitudeA < _config.deadbandA)
        {
            _measurement.currentA = 0.0f;
            _measurement.measurementStatus = iotsmartsys::core::CurrentMeasurementStatus::ESTIMATED;
        }
        else if (magnitudeA < _config.minimumReportableA)
        {
            _measurement.currentA = currentA;
            _measurement.measurementStatus = iotsmartsys::core::CurrentMeasurementStatus::ESTIMATED;
        }
        else if (magnitudeA <= _config.maximumAbsoluteCurrentA)
        {
            _measurement.currentA = currentA;
            _measurement.measurementStatus = iotsmartsys::core::CurrentMeasurementStatus::VALID;
        }
        else
        {
            _measurement.currentA.reset();
            _measurement.measurementStatus =
                iotsmartsys::core::CurrentMeasurementStatus::OVERCURRENT_OR_SATURATION;
        }

        _lastStateReadMs = nowMs;
        _lastMeasurementCompletedMs = nowMs;
        _hasCompletedMeasurement = true;
    }

    long INA3221CurrentSensor::lastStateReadMillis() const
    {
        return static_cast<long>(_lastStateReadMs);
    }

    const iotsmartsys::core::CurrentMeasurement &INA3221CurrentSensor::currentMeasurement() const
    {
        return _measurement;
    }

    std::optional<float> INA3221CurrentSensor::calibratedZeroAdcMv() const
    {
        return std::nullopt;
    }

    void INA3221CurrentSensor::requestZeroCalibration()
    {
        iotsmartsys::core::Log::get().warn(
            kLogTag, "Zero calibration is not applicable to INA3221 shunt measurements.");
    }

    bool INA3221CurrentSensor::configValid() const
    {
        if (_config.channel > 2 || _config.readingIntervalMs == 0 ||
            !std::isfinite(_config.shuntResistanceOhms) || _config.shuntResistanceOhms <= 0.0f ||
            (_config.polarity != 1.0f && _config.polarity != -1.0f) ||
            !std::isfinite(_config.deadbandA) || _config.deadbandA < 0.0f ||
            !std::isfinite(_config.minimumReportableA) || _config.minimumReportableA < 0.0f ||
            !std::isfinite(_config.maximumAbsoluteCurrentA) ||
            _config.maximumAbsoluteCurrentA <= 0.0f)
        {
            return false;
        }

        const float maximumMeasurableCurrentA = 0.1638f / _config.shuntResistanceOhms;
        return std::isfinite(maximumMeasurableCurrentA) &&
               _config.maximumAbsoluteCurrentA <= maximumMeasurableCurrentA &&
               _config.deadbandA <= _config.minimumReportableA &&
               _config.minimumReportableA <= _config.maximumAbsoluteCurrentA;
    }
}
