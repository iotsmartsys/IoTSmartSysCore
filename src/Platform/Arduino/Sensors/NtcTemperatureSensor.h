#pragma once

#include <cstddef>
#include <cstdint>

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
        float adcReferenceVoltageV{3.3f};
        std::uint8_t adcResolutionBits{12};

        static NtcTemperatureSensorConfig NTC_100K_B3950(int adcPin);
        static NtcTemperatureSensorConfig MF52_103_B3950(int adcPin);

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
