#pragma once

#include <cstddef>
#include <cstdint>

#include "Platform/Arduino/Sensors/Esp32Adc1.h"

#include "Contracts/Sensors/ITemperatureSensor.h"

namespace iotsmartsys::platform::arduino
{
    struct NtcTemperatureSensorConfig
    {
        int adcPin{-1};
        float nominalResistanceOhms{0.0f};
        float betaK{0.0f};
        float referenceTemperatureC{25.0f};
        float seriesResistanceOhms{0.0f};
        float supplyVoltageV{3.3f};
        // Selects attenuation only; calibrated millivolts are not scaled by this value.
        float adcReferenceVoltageV{3.3f};
        std::uint8_t adcResolutionBits{12};

        static NtcTemperatureSensorConfig NTC_100K_B3950(Esp32Adc1Pin adcPin);
        static NtcTemperatureSensorConfig MF52_103_B3950(Esp32Adc1Pin adcPin);

        bool isValid() const;
    };

    class NtcTemperatureSensor final : public iotsmartsys::core::ITemperatureSensor
    {
    public:
        static constexpr float INVALID_TEMPERATURE_C = -1000.0f;
        static constexpr std::size_t SAMPLES_PER_READING = 16;

        explicit NtcTemperatureSensor(const NtcTemperatureSensorConfig &config);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;
        float readTemperatureCelsius() override;

        static bool isSupportedAdcPin(int pin);
        static bool isSupportedConfig(const NtcTemperatureSensorConfig &config);

    private:
        float invalidReading(const char *reason);

        NtcTemperatureSensorConfig _config;
        long _lastStateReadMillis{0};
        bool _configSupported{false};
        bool _setupComplete{false};
    };
}
