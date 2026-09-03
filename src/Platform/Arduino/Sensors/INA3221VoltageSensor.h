#pragma once

#include <cstdint>

#include "Contracts/Sensors/IVoltageSensor.h"
#include "Platform/Arduino/Sensors/INA3221Device.h"

namespace iotsmartsys::platform::arduino
{
    struct INA3221VoltageSensorConfig
    {
        std::uint8_t channel{3};
        float minimumVoltageV{0.0f};
        float maximumVoltageV{26.0f};
        std::uint32_t readingIntervalMs{500};
    };

    class INA3221VoltageSensor final : public iotsmartsys::core::IVoltageSensor
    {
    public:
        INA3221VoltageSensor(INA3221Device &device, const INA3221VoltageSensorConfig &config);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;
        const iotsmartsys::core::VoltageMeasurement &voltageMeasurement() const override;

    private:
        bool configValid() const;

        INA3221Device &_device;
        INA3221VoltageSensorConfig _config;
        iotsmartsys::core::VoltageMeasurement _measurement;
        std::uint32_t _lastMeasurementCompletedMs{0};
        unsigned long _lastStateReadMs{0};
        bool _setupComplete{false};
        bool _hasCompletedMeasurement{false};
    };
}
