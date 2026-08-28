#include "Platform/Arduino/Sensors/NtcTemperatureSensor.h"

#include <Arduino.h>
#include <cmath>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    namespace
    {
        constexpr const char *kLogTag = "NTC_TEMPERATURE_SENSOR";
        constexpr double kKelvinOffset = 273.15;

        bool finitePositive(float value)
        {
            return std::isfinite(value) && value > 0.0f;
        }

        adc_attenuation_t attenuationFor(float referenceVoltageV)
        {
            if (referenceVoltageV <= 0.95f)
                return ADC_0db;
            if (referenceVoltageV <= 1.25f)
                return ADC_2_5db;
            if (referenceVoltageV <= 1.75f)
                return ADC_6db;
            return ADC_11db;
        }
    }

    NtcTemperatureSensorConfig NtcTemperatureSensorConfig::NTC_100K_B3950(int pin)
    {
        NtcTemperatureSensorConfig config;
        config.adcPin = pin;
        config.nominalResistanceOhms = 100000.0f;
        config.betaK = 3950.0f;
        config.referenceTemperatureC = 25.0f;
        config.seriesResistanceOhms = 100000.0f;
        return config;
    }

    NtcTemperatureSensorConfig NtcTemperatureSensorConfig::MF52_103_B3950(int pin)
    {
        NtcTemperatureSensorConfig config;
        config.adcPin = pin;
        config.nominalResistanceOhms = 10000.0f;
        config.betaK = 3950.0f;
        config.referenceTemperatureC = 25.0f;
        config.seriesResistanceOhms = 10000.0f;
        return config;
    }

    bool NtcTemperatureSensorConfig::isValid() const
    {
        const double referenceTemperatureK =
            static_cast<double>(referenceTemperatureC) + kKelvinOffset;
        return adcPin >= 0 &&
               finitePositive(nominalResistanceOhms) &&
               finitePositive(betaK) &&
               std::isfinite(referenceTemperatureC) &&
               std::isfinite(referenceTemperatureK) &&
               referenceTemperatureK > 0.0 &&
               finitePositive(seriesResistanceOhms) &&
               finitePositive(supplyVoltageV) &&
               finitePositive(adcReferenceVoltageV) &&
               adcReferenceVoltageV <= 3.3f &&
               adcResolutionBits >= 9 && adcResolutionBits <= 12;
    }

    NtcTemperatureSensor::NtcTemperatureSensor(const NtcTemperatureSensorConfig &config)
        : _config(config),
          _configSupported(isSupportedConfig(config))
    {
    }

    bool NtcTemperatureSensor::isSupportedAdcPin(int pin)
    {
#if defined(CONFIG_IDF_TARGET_ESP32)
        return pin >= 32 && pin <= 39 && pin < NUM_DIGITAL_PINS &&
               digitalPinToAnalogChannel(static_cast<std::uint8_t>(pin)) >= 0;
#else
        (void)pin;
        return false;
#endif
    }

    bool NtcTemperatureSensor::isSupportedConfig(const NtcTemperatureSensorConfig &config)
    {
        return config.isValid() && isSupportedAdcPin(config.adcPin);
    }

    void NtcTemperatureSensor::setup()
    {
        _lastStateReadMillis = 0;
        _setupComplete = false;
        if (!_configSupported)
        {
            iotsmartsys::core::Log::get().error(
                kLogTag,
                "Invalid NTC configuration: pin=%d R0=%.2f beta=%.2f series=%.2f.",
                _config.adcPin,
                static_cast<double>(_config.nominalResistanceOhms),
                static_cast<double>(_config.betaK),
                static_cast<double>(_config.seriesResistanceOhms));
            return;
        }

        analogReadResolution(_config.adcResolutionBits);
        analogSetPinAttenuation(
            static_cast<std::uint8_t>(_config.adcPin),
            attenuationFor(_config.adcReferenceVoltageV));
        pinMode(_config.adcPin, INPUT);
        _setupComplete = true;

        iotsmartsys::core::Log::get().info(
            kLogTag,
            "Configured NTC: pin=%d R0=%.2f beta=%.2f T0=%.2f series=%.2f samples=%u.",
            _config.adcPin,
            static_cast<double>(_config.nominalResistanceOhms),
            static_cast<double>(_config.betaK),
            static_cast<double>(_config.referenceTemperatureC),
            static_cast<double>(_config.seriesResistanceOhms),
            static_cast<unsigned>(SAMPLES_PER_READING));
    }

    void NtcTemperatureSensor::handle()
    {
    }

    long NtcTemperatureSensor::lastStateReadMillis() const
    {
        return _lastStateReadMillis;
    }

    float NtcTemperatureSensor::readTemperatureCelsius()
    {
        if (!_configSupported)
            return invalidReading("invalid configuration");
        if (!_setupComplete)
            return invalidReading("setup not completed");

        std::uint32_t adcSum = 0;
        for (std::size_t sample = 0; sample < SAMPLES_PER_READING; ++sample)
            adcSum += analogRead(static_cast<std::uint8_t>(_config.adcPin));

        _lastStateReadMillis = static_cast<long>(millis());
        const double adcAverage =
            static_cast<double>(adcSum) / static_cast<double>(SAMPLES_PER_READING);
        const double adcMaximum =
            static_cast<double>((std::uint32_t{1} << _config.adcResolutionBits) - 1U);
        if (adcAverage <= 0.0 || adcAverage >= adcMaximum)
            return invalidReading("ADC at range boundary");

        const double adcVoltage =
            (adcAverage / adcMaximum) * static_cast<double>(_config.adcReferenceVoltageV);
        const double denominator = static_cast<double>(_config.supplyVoltageV) - adcVoltage;
        if (!std::isfinite(adcVoltage) || adcVoltage >= _config.supplyVoltageV ||
            !std::isfinite(denominator) || denominator <= 0.0)
        {
            return invalidReading("invalid divider voltage");
        }

        const double ntcResistance =
            static_cast<double>(_config.seriesResistanceOhms) * adcVoltage / denominator;
        if (!std::isfinite(ntcResistance) || ntcResistance <= 0.0)
            return invalidReading("invalid NTC resistance");

        const double resistanceRatio =
            ntcResistance / static_cast<double>(_config.nominalResistanceOhms);
        if (!std::isfinite(resistanceRatio) || resistanceRatio <= 0.0)
            return invalidReading("invalid logarithm argument");

        const double referenceTemperatureK =
            static_cast<double>(_config.referenceTemperatureC) + kKelvinOffset;
        const double inverseTemperatureK =
            (1.0 / referenceTemperatureK) +
            (std::log(resistanceRatio) / static_cast<double>(_config.betaK));
        if (!std::isfinite(inverseTemperatureK) || inverseTemperatureK <= 0.0)
            return invalidReading("invalid inverse temperature");

        const double temperatureK = 1.0 / inverseTemperatureK;
        const double temperatureC = temperatureK - kKelvinOffset;
        if (!std::isfinite(temperatureK) || temperatureK <= 0.0 ||
            !std::isfinite(temperatureC))
        {
            return invalidReading("invalid temperature result");
        }

        iotsmartsys::core::Log::get().debug(
            kLogTag,
            "NTC reading: adc=%.2f voltage=%.4fV resistance=%.2fohm temperature=%.2fC.",
            adcAverage,
            adcVoltage,
            ntcResistance,
            temperatureC);
        return static_cast<float>(temperatureC);
    }

    float NtcTemperatureSensor::invalidReading(const char *reason)
    {
        _lastStateReadMillis = static_cast<long>(millis());
        iotsmartsys::core::Log::get().warn(
            kLogTag,
            "Invalid NTC reading: reason=%s sentinel=%.1fC.",
            reason,
            static_cast<double>(INVALID_TEMPERATURE_C));
        return INVALID_TEMPERATURE_C;
    }
}
