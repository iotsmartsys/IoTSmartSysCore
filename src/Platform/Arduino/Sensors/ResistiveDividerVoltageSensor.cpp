#include "Platform/Arduino/Sensors/ResistiveDividerVoltageSensor.h"

#include <Arduino.h>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    using iotsmartsys::core::Log;
    using iotsmartsys::core::VoltageMeasurementStatus;

    namespace
    {
        constexpr const char *kLogTag = "VOLTAGE_SENSOR";
    }

    ResistiveDividerVoltageSensor::ResistiveDividerVoltageSensor(
        const iotsmartsys::core::VoltageSensorConfig &config)
        : _config(config),
          _dividerRatio((static_cast<double>(config.r1Ohms) + config.r2Ohms) / config.r2Ohms)
    {
    }

    bool ResistiveDividerVoltageSensor::elapsed(std::uint32_t now,
                                                std::uint32_t since,
                                                std::uint32_t interval)
    {
        return static_cast<std::uint32_t>(now - since) >= interval;
    }

    void ResistiveDividerVoltageSensor::setup()
    {
        analogReadResolution(_config.adcResolutionBits);
        analogSetPinAttenuation(static_cast<std::uint8_t>(_config.adcPin), ADC_11db);
        pinMode(_config.adcPin, INPUT);

        _measurement.voltageV.reset();
        _measurement.measurementStatus = VoltageMeasurementStatus::NOT_READY;
        _lastStateReadMs = 0;
        _hasSample = false;
        _hasCompletedMeasurement = false;
        resetAccumulator();
        _setupComplete = true;

        Log::get().info(kLogTag,
                        "Voltage sensor '%s' configured: pin=%d, ratio=%.6f, minimum=%.1f mV.",
                        _config.id.c_str(),
                        _config.adcPin,
                        _dividerRatio,
                        static_cast<double>(_config.adcMinimumMv));
    }

    void ResistiveDividerVoltageSensor::handle()
    {
        if (!_setupComplete)
        {
            return;
        }

        const std::uint32_t nowMs = millis();
        const std::uint32_t nowUs = micros();
        if (!_batchActive)
        {
            if (_hasCompletedMeasurement &&
                !elapsed(nowMs, _lastMeasurementCompletedMs, _config.readingIntervalMs))
            {
                return;
            }
            _batchActive = true;
        }

        if (!sampleEligible(nowUs))
        {
            return;
        }

        _lastSampleUs = nowUs;
        _hasSample = true;
        _sampleSumMv += analogReadMilliVolts(static_cast<std::uint8_t>(_config.adcPin));
        _sampleCount++;
        if (_sampleCount >= _config.samplesPerReading)
        {
            completeMeasurement(nowMs);
        }
    }

    long ResistiveDividerVoltageSensor::lastStateReadMillis() const
    {
        return static_cast<long>(_lastStateReadMs);
    }

    const iotsmartsys::core::VoltageMeasurement &
    ResistiveDividerVoltageSensor::voltageMeasurement() const
    {
        return _measurement;
    }

    bool ResistiveDividerVoltageSensor::sampleEligible(std::uint32_t nowUs) const
    {
        return !_hasSample || elapsed(nowUs, _lastSampleUs, _config.sampleIntervalUs);
    }

    void ResistiveDividerVoltageSensor::completeMeasurement(std::uint32_t nowMs)
    {
        const double averageMv = _sampleSumMv / static_cast<double>(_sampleCount);
        if (averageMv < _config.adcMinimumMv)
        {
            _measurement.voltageV = -1000.0f;
            _measurement.measurementStatus = VoltageMeasurementStatus::BELOW_MINIMUM;
        }
        else if (averageMv >= _config.adcMaximumMv)
        {
            _measurement.voltageV.reset();
            _measurement.measurementStatus = VoltageMeasurementStatus::ADC_SATURATION;
        }
        else
        {
            _measurement.voltageV = static_cast<float>((averageMv / 1000.0) * _dividerRatio);
            _measurement.measurementStatus = VoltageMeasurementStatus::VALID;
        }

        _lastStateReadMs = nowMs;
        _lastMeasurementCompletedMs = nowMs;
        _hasCompletedMeasurement = true;
        Log::get().debug(kLogTag,
                         "Voltage '%s': average=%.2f mV, voltage=%s, status=%s.",
                         _config.id.c_str(),
                         averageMv,
                         _measurement.voltageV ? "present" : "unavailable",
                         iotsmartsys::core::toString(_measurement.measurementStatus));
        resetAccumulator();
    }

    void ResistiveDividerVoltageSensor::resetAccumulator()
    {
        _sampleSumMv = 0.0;
        _sampleCount = 0;
        _batchActive = false;
    }
}
