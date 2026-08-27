#include "Platform/Arduino/Sensors/ACS712C30ACurrentSensor.h"

#include <Arduino.h>
#include <cmath>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    using iotsmartsys::core::CurrentMeasurementStatus;
    using iotsmartsys::core::CurrentSensorQualification;
    using iotsmartsys::core::CurrentSupplyStatus;
    using iotsmartsys::core::Log;

    namespace
    {
        constexpr const char *kLogTag = "ACS712C30A";

        bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval)
        {
            return static_cast<std::uint32_t>(now - since) >= interval;
        }
    }

    ACS712C30ACurrentSensor::ACS712C30ACurrentSensor(const iotsmartsys::core::CurrentSensorConfig &config)
        : _config(config)
    {
        _measurement.measurementStatus = CurrentMeasurementStatus::NOT_READY;
        _measurement.supplyStatus = config.supplyMonitorAdcPin >= 0
                                        ? CurrentSupplyStatus::UNKNOWN
                                        : CurrentSupplyStatus::NOT_MONITORED;
    }

    void ACS712C30ACurrentSensor::setup()
    {
        analogReadResolution(_config.adcResolutionBits);
        analogSetPinAttenuation(static_cast<std::uint8_t>(_config.adcPin), ADC_11db);
        pinMode(_config.adcPin, INPUT);

        if (_config.supplyMonitorAdcPin >= 0)
        {
            analogSetPinAttenuation(static_cast<std::uint8_t>(_config.supplyMonitorAdcPin), ADC_11db);
            pinMode(_config.supplyMonitorAdcPin, INPUT);
        }

        _setupStartedMs = millis();
        _phaseStartedMs = _setupStartedMs;
        _phase = Phase::WARMUP;
        _measurement.currentA.reset();
        _measurement.measurementStatus = CurrentMeasurementStatus::NOT_READY;
        _measurement.supplyStatus = _config.supplyMonitorAdcPin >= 0
                                        ? CurrentSupplyStatus::UNKNOWN
                                        : CurrentSupplyStatus::NOT_MONITORED;
        _calibratedZeroMv.reset();
        _hasValidZero = false;
        _recalibrationRequested = false;
        _hasAdcRead = false;
        _hasSupplySample = false;
        _lastStateReadMs = 0;
        _filterInitialized = false;
        resetAccumulator();
        _setupComplete = true;

        Log::get().info(kLogTag,
                        "Current sensor '%s' configured: supply=%u mV, qualification=%s.",
                        _config.id.c_str(),
                        static_cast<unsigned>(_config.supplyNominalMv),
                        _config.qualification == CurrentSensorQualification::MANUFACTURER_SUPPORTED
                            ? "MANUFACTURER_SUPPORTED"
                            : "PROJECT_VALIDATED");
        if (_config.supplyMonitorAdcPin < 0)
        {
            Log::get().warn(kLogTag, "Supply for '%s' is not monitored.", _config.id.c_str());
        }
    }

    void ACS712C30ACurrentSensor::handle()
    {
        if (!_setupComplete)
        {
            return;
        }

        const std::uint32_t nowMs = millis();
        const std::uint32_t nowUs = micros();

        if (_phase != Phase::CALIBRATING && _phase != Phase::RECALIBRATION_SETTLE &&
            _recalibrationRequested)
        {
            if (_measurement.supplyStatus == CurrentSupplyStatus::UNKNOWN)
            {
                sampleSupply(nowMs, nowUs);
                return;
            }

            _recalibrationRequested = false;
            if (!supplyAllowsCalibration())
            {
                Log::get().warn(kLogTag,
                                "Zero recalibration for '%s' rejected: supply prerequisite not met.",
                                _config.id.c_str());
                return;
            }

            _measurementBeforeCalibration = _measurement;
            _calibrationIsInitial = false;
            _measurement.currentA.reset();
            _measurement.measurementStatus = CurrentMeasurementStatus::CALIBRATING;
            _phase = Phase::RECALIBRATION_SETTLE;
            _phaseStartedMs = nowMs;
            Log::get().info(kLogTag, "Zero recalibration for '%s' started.", _config.id.c_str());
            return;
        }

        if (_phase == Phase::WARMUP)
        {
            if (sampleSupply(nowMs, nowUs))
            {
                return;
            }
            if (elapsed(nowMs, _setupStartedMs, _config.startupWarmupMs) && supplyAllowsCalibration())
            {
                beginCalibration(true, nowMs);
            }
            return;
        }

        if (_phase == Phase::RECALIBRATION_SETTLE)
        {
            if (sampleSupply(nowMs, nowUs))
            {
                if (!supplyAllowsCalibration())
                {
                    abortCalibrationForSupply();
                }
                return;
            }
            if (!supplyAllowsCalibration())
            {
                abortCalibrationForSupply();
                return;
            }
            if (elapsed(nowMs, _phaseStartedMs, _config.recalibrationSettleMs))
            {
                beginCalibration(false, nowMs);
            }
            return;
        }

        if (_phase == Phase::CALIBRATING)
        {
            if (sampleSupply(nowMs, nowUs))
            {
                if (!supplyAllowsCalibration())
                {
                    abortCalibrationForSupply();
                }
                return;
            }
            if (!supplyAllowsCalibration())
            {
                abortCalibrationForSupply();
                return;
            }
            collectCalibrationSample(nowMs, nowUs);
            return;
        }

        if (_phase == Phase::ZERO_FAILED)
        {
            sampleSupply(nowMs, nowUs);
            return;
        }

        if (sampleSupply(nowMs, nowUs))
        {
            return;
        }
        collectMeasurementSample(nowMs, nowUs);
    }

    long ACS712C30ACurrentSensor::lastStateReadMillis() const
    {
        return static_cast<long>(_lastStateReadMs);
    }

    const iotsmartsys::core::CurrentMeasurement &ACS712C30ACurrentSensor::currentMeasurement() const
    {
        return _measurement;
    }

    std::optional<float> ACS712C30ACurrentSensor::calibratedZeroAdcMv() const
    {
        return _calibratedZeroMv;
    }

    void ACS712C30ACurrentSensor::requestZeroCalibration()
    {
        if (_phase == Phase::CALIBRATING || _phase == Phase::RECALIBRATION_SETTLE ||
            (_phase == Phase::WARMUP && !_hasValidZero))
        {
            Log::get().warn(kLogTag,
                            "Concurrent zero calibration request for '%s' discarded.",
                            _config.id.c_str());
            return;
        }
        _recalibrationRequested = true;
    }

    bool ACS712C30ACurrentSensor::adcOpportunityEligible(std::uint32_t nowUs) const
    {
        return !_hasAdcRead || elapsed(nowUs, _lastAdcReadUs, _config.sampleIntervalUs);
    }

    std::uint32_t ACS712C30ACurrentSensor::readAdcMilliVolts(int pin, std::uint32_t nowUs)
    {
        _lastAdcReadUs = nowUs;
        _hasAdcRead = true;
        return analogReadMilliVolts(static_cast<std::uint8_t>(pin));
    }

    bool ACS712C30ACurrentSensor::supplySampleDue(std::uint32_t nowMs) const
    {
        return _config.supplyMonitorAdcPin >= 0 &&
               (!_hasSupplySample || elapsed(nowMs, _lastSupplySampleMs, _config.readingIntervalMs));
    }

    bool ACS712C30ACurrentSensor::sampleSupply(std::uint32_t nowMs, std::uint32_t nowUs)
    {
        if (!supplySampleDue(nowMs) || !adcOpportunityEligible(nowUs))
        {
            return false;
        }

        const std::uint32_t adcMv = readAdcMilliVolts(_config.supplyMonitorAdcPin, nowUs);
        const float supplyMv = static_cast<float>(adcMv) / _config.supplyMonitorToVccRatio;
        _hasSupplySample = true;
        _lastSupplySampleMs = nowMs;
        _measurement.supplyStatus = supplyMv >= _config.supplyValidMinimumMv &&
                                            supplyMv <= _config.supplyValidMaximumMv
                                        ? CurrentSupplyStatus::IN_RANGE
                                        : CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE;
        if (_phase != Phase::WARMUP)
        {
            _lastStateReadMs = nowMs;
        }

        if (_measurement.supplyStatus == CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE)
        {
            _measurement.currentA.reset();
            Log::get().error(kLogTag,
                             "Supply for '%s' is out of range: %.1f mV.",
                             _config.id.c_str(),
                             static_cast<double>(supplyMv));
        }
        else
        {
            Log::get().debug(kLogTag,
                             "Supply for '%s': %.1f mV.",
                             _config.id.c_str(),
                             static_cast<double>(supplyMv));
        }
        return true;
    }

    bool ACS712C30ACurrentSensor::supplyAllowsCalibration() const
    {
        return _measurement.supplyStatus == CurrentSupplyStatus::IN_RANGE ||
               _measurement.supplyStatus == CurrentSupplyStatus::NOT_MONITORED;
    }

    void ACS712C30ACurrentSensor::beginCalibration(bool initial, std::uint32_t nowMs)
    {
        _calibrationIsInitial = initial;
        _measurement.currentA.reset();
        _measurement.measurementStatus = CurrentMeasurementStatus::CALIBRATING;
        _phase = Phase::CALIBRATING;
        _phaseStartedMs = nowMs;
        resetAccumulator();
        Log::get().info(kLogTag,
                        "%s zero calibration for '%s' is sampling.",
                        initial ? "Initial" : "Requested",
                        _config.id.c_str());
    }

    void ACS712C30ACurrentSensor::abortCalibrationForSupply()
    {
        resetAccumulator();
        if (_calibrationIsInitial)
        {
            _phase = Phase::WARMUP;
            _measurement.currentA.reset();
            _measurement.measurementStatus = CurrentMeasurementStatus::NOT_READY;
        }
        else
        {
            const CurrentSupplyStatus supplyStatus = _measurement.supplyStatus;
            _measurement = _measurementBeforeCalibration;
            _measurement.supplyStatus = supplyStatus;
            if (supplyStatus == CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE ||
                supplyStatus == CurrentSupplyStatus::UNKNOWN)
            {
                _measurement.currentA.reset();
            }
            _phase = _measurement.measurementStatus == CurrentMeasurementStatus::ZERO_CALIBRATION_FAILED
                         ? Phase::ZERO_FAILED
                         : Phase::MEASURING;
        }
        Log::get().warn(kLogTag,
                        "Zero calibration for '%s' stopped: supply prerequisite lost.",
                        _config.id.c_str());
    }

    void ACS712C30ACurrentSensor::resetAccumulator()
    {
        _sampleSumMv = 0.0;
        _sampleCount = 0;
        _sampleSaturated = false;
        _batchActive = false;
    }

    void ACS712C30ACurrentSensor::collectCalibrationSample(std::uint32_t nowMs, std::uint32_t nowUs)
    {
        if (!adcOpportunityEligible(nowUs))
        {
            return;
        }

        const std::uint32_t milliVolts = readAdcMilliVolts(_config.adcPin, nowUs);
        _sampleSumMv += milliVolts;
        _sampleCount++;
        _sampleSaturated = _sampleSaturated ||
                           milliVolts <= _config.adcMinimumMv ||
                           milliVolts >= _config.adcMaximumMv;
        if (_sampleCount >= _config.zeroCalibrationSamples)
        {
            completeCalibration(nowMs);
        }
    }

    void ACS712C30ACurrentSensor::completeCalibration(std::uint32_t nowMs)
    {
        const float zeroMv = static_cast<float>(_sampleSumMv / static_cast<double>(_sampleCount));
        const bool valid = !_sampleSaturated &&
                           std::fabs(zeroMv - _config.nominalZeroAdcMv) <= _config.maximumZeroDeviationMv;
        resetAccumulator();

        if (!valid)
        {
            _calibratedZeroMv.reset();
            _hasValidZero = false;
            _measurement.currentA.reset();
            _measurement.measurementStatus = CurrentMeasurementStatus::ZERO_CALIBRATION_FAILED;
            _phase = Phase::ZERO_FAILED;
            _lastStateReadMs = nowMs;
            Log::get().error(kLogTag,
                             "Zero calibration for '%s' failed: %.2f mV (nominal %.2f mV).",
                             _config.id.c_str(),
                             static_cast<double>(zeroMv),
                             static_cast<double>(_config.nominalZeroAdcMv));
            return;
        }

        _calibratedZeroMv = zeroMv;
        _hasValidZero = true;
        _filterInitialized = false;
        _measurement.measurementStatus = CurrentMeasurementStatus::ESTIMATED;
        if (_measurement.supplyStatus == CurrentSupplyStatus::IN_RANGE ||
            _measurement.supplyStatus == CurrentSupplyStatus::NOT_MONITORED)
        {
            _measurement.currentA = 0.0f;
        }
        else
        {
            _measurement.currentA.reset();
        }
        _phase = Phase::MEASURING;
        _lastStateReadMs = nowMs;
        _lastMeasurementCompletedMs = nowMs;
        Log::get().info(kLogTag,
                        "Zero calibration for '%s' completed: %.2f mV.",
                        _config.id.c_str(),
                        static_cast<double>(zeroMv));
    }

    void ACS712C30ACurrentSensor::collectMeasurementSample(std::uint32_t nowMs, std::uint32_t nowUs)
    {
        if (!_batchActive)
        {
            if (!elapsed(nowMs, _lastMeasurementCompletedMs, _config.readingIntervalMs))
            {
                return;
            }
            _batchActive = true;
        }

        if (!adcOpportunityEligible(nowUs))
        {
            return;
        }

        const std::uint32_t milliVolts = readAdcMilliVolts(_config.adcPin, nowUs);
        _sampleSumMv += milliVolts;
        _sampleCount++;
        _sampleSaturated = _sampleSaturated ||
                           milliVolts <= _config.adcMinimumMv ||
                           milliVolts >= _config.adcMaximumMv;
        if (_sampleCount >= _config.samplesPerReading)
        {
            completeMeasurement(nowMs);
        }
    }

    void ACS712C30ACurrentSensor::completeMeasurement(std::uint32_t nowMs)
    {
        const float averageMv = static_cast<float>(_sampleSumMv / static_cast<double>(_sampleCount));
        if (!_filterInitialized)
        {
            _filteredMv = averageMv;
            _filterInitialized = true;
        }
        else
        {
            _filteredMv = _config.lowPassAlpha * averageMv +
                          (1.0f - _config.lowPassAlpha) * _filteredMv;
        }

        qualifyCurrent(_filteredMv);
        _lastStateReadMs = nowMs;
        _lastMeasurementCompletedMs = nowMs;
        Log::get().debug(kLogTag,
                         "Current '%s': average=%.2f mV, zero=%.2f mV, current=%s, measurement=%s, supply=%s.",
                         _config.id.c_str(),
                         static_cast<double>(averageMv),
                         static_cast<double>(_calibratedZeroMv.value_or(_config.nominalZeroAdcMv)),
                         _measurement.currentA ? "present" : "unavailable",
                         iotsmartsys::core::toString(_measurement.measurementStatus),
                         iotsmartsys::core::toString(_measurement.supplyStatus));
        resetAccumulator();
    }

    void ACS712C30ACurrentSensor::qualifyCurrent(float filteredMilliVolts)
    {
        if (_sampleSaturated || !_calibratedZeroMv)
        {
            _measurement.currentA.reset();
            _measurement.measurementStatus = CurrentMeasurementStatus::OVERCURRENT_OR_SATURATION;
            Log::get().error(kLogTag, "Current signal for '%s' is saturated.", _config.id.c_str());
            return;
        }

        float currentA = _config.polarity *
                         (filteredMilliVolts - *_calibratedZeroMv) /
                         _config.sensitivityAdcMvPerA;
        const float magnitudeA = std::fabs(currentA);

        if (currentA < _config.physicalMinimumA || currentA > _config.physicalMaximumA)
        {
            _measurement.currentA.reset();
            _measurement.measurementStatus = CurrentMeasurementStatus::OVERCURRENT_OR_SATURATION;
            Log::get().error(kLogTag, "Overcurrent for '%s': %.3f A.", _config.id.c_str(), static_cast<double>(currentA));
            return;
        }

        if (magnitudeA > _config.calibratedMaximumA)
        {
            _measurement.currentA.reset();
            _measurement.measurementStatus = CurrentMeasurementStatus::OUT_OF_CALIBRATED_RANGE;
            Log::get().warn(kLogTag,
                            "Current for '%s' is outside the calibrated range: %.3f A.",
                            _config.id.c_str(),
                            static_cast<double>(currentA));
            return;
        }

        if (magnitudeA < _config.deadbandA || magnitudeA < _config.minimumReportableA)
        {
            currentA = 0.0f;
            _measurement.measurementStatus = CurrentMeasurementStatus::ESTIMATED;
        }
        else if (magnitudeA < _config.calibratedMinimumA)
        {
            _measurement.measurementStatus = CurrentMeasurementStatus::ESTIMATED;
        }
        else
        {
            _measurement.measurementStatus = CurrentMeasurementStatus::VALID;
        }

        if (_measurement.supplyStatus == CurrentSupplyStatus::UNKNOWN ||
            _measurement.supplyStatus == CurrentSupplyStatus::SUPPLY_OUT_OF_RANGE)
        {
            _measurement.currentA.reset();
        }
        else
        {
            _measurement.currentA = currentA;
        }
    }
}
