#include "Platform/Arduino/Sensors/INA3221VoltageSensor.h"

#include <Arduino.h>
#include <cmath>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    namespace
    {
        constexpr const char *kLogTag = "INA3221_VOLTAGE";

        bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval)
        {
            return static_cast<std::uint32_t>(now - since) >= interval;
        }
    }

    INA3221VoltageSensor::INA3221VoltageSensor(
        INA3221Device &device, const INA3221VoltageSensorConfig &config)
        : _device(device), _config(config)
    {
    }

    void INA3221VoltageSensor::setup()
    {
        _measurement.voltageV.reset();
        _measurement.measurementStatus = iotsmartsys::core::VoltageMeasurementStatus::NOT_READY;
        _lastStateReadMs = 0;
        _lastMeasurementCompletedMs = 0;
        _hasCompletedMeasurement = false;
        _setupComplete = false;

        if (!configValid())
        {
            iotsmartsys::core::Log::get().error(kLogTag, "Invalid voltage sensor configuration.");
            return;
        }
        _setupComplete = _device.setup();
        if (!_setupComplete)
        {
            iotsmartsys::core::Log::get().error(kLogTag, "Shared device is unavailable.");
        }
    }

    void INA3221VoltageSensor::handle()
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

        const float voltageV = _device.busVoltage(_config.channel);
        if (!std::isfinite(voltageV))
        {
            _measurement.voltageV.reset();
            _measurement.measurementStatus = iotsmartsys::core::VoltageMeasurementStatus::NOT_READY;
            return;
        }

        if (voltageV < _config.minimumVoltageV)
        {
            _measurement.voltageV = -1000.0f;
            _measurement.measurementStatus = iotsmartsys::core::VoltageMeasurementStatus::BELOW_MINIMUM;
        }
        else if (voltageV >= _config.maximumVoltageV)
        {
            _measurement.voltageV.reset();
            _measurement.measurementStatus = iotsmartsys::core::VoltageMeasurementStatus::ADC_SATURATION;
        }
        else
        {
            _measurement.voltageV = voltageV;
            _measurement.measurementStatus = iotsmartsys::core::VoltageMeasurementStatus::VALID;
        }

        _lastStateReadMs = nowMs;
        _lastMeasurementCompletedMs = nowMs;
        _hasCompletedMeasurement = true;
    }

    long INA3221VoltageSensor::lastStateReadMillis() const
    {
        return static_cast<long>(_lastStateReadMs);
    }

    const iotsmartsys::core::VoltageMeasurement &INA3221VoltageSensor::voltageMeasurement() const
    {
        return _measurement;
    }

    bool INA3221VoltageSensor::configValid() const
    {
        return _config.channel <= 2 && _config.readingIntervalMs > 0 &&
               std::isfinite(_config.minimumVoltageV) && _config.minimumVoltageV >= 0.0f &&
               std::isfinite(_config.maximumVoltageV) &&
               _config.maximumVoltageV > _config.minimumVoltageV &&
               _config.maximumVoltageV <= 26.0f;
    }
}
