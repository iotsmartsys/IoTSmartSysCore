#pragma once

#include <cstdint>
#include <optional>

#include "Contracts/Sensors/ICurrentSensor.h"
#include "Platform/Arduino/Sensors/INA3221Device.h"

namespace iotsmartsys::platform::arduino
{
    struct INA3221CurrentSensorConfig
    {
        std::uint8_t channel{3};
        float shuntResistanceOhms{0.0f};
        float polarity{1.0f};
        float deadbandA{0.0f};
        float minimumReportableA{0.0f};
        float maximumAbsoluteCurrentA{0.0f};
        std::uint32_t readingIntervalMs{500};
    };

    class INA3221CurrentSensor final : public iotsmartsys::core::ICurrentSensor
    {
    public:
        INA3221CurrentSensor(INA3221Device &device, const INA3221CurrentSensorConfig &config);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;
        const iotsmartsys::core::CurrentMeasurement &currentMeasurement() const override;
        std::optional<float> calibratedZeroAdcMv() const override;
        void requestZeroCalibration() override;

    private:
        bool configValid() const;

        INA3221Device &_device;
        INA3221CurrentSensorConfig _config;
        iotsmartsys::core::CurrentMeasurement _measurement;
        std::uint32_t _lastMeasurementCompletedMs{0};
        unsigned long _lastStateReadMs{0};
        bool _setupComplete{false};
        bool _hasCompletedMeasurement{false};
    };
}
