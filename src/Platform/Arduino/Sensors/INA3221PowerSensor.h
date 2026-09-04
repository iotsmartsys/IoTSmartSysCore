#pragma once

#include <cstdint>

#include "Contracts/Sensors/IPowerSensor.h"
#include "Platform/Arduino/Sensors/INA3221Device.h"

namespace iotsmartsys::platform::arduino
{
    struct INA3221PowerSensorConfig
    {
        std::uint8_t channel{3};
        float shuntResistanceOhms{0.0f};
        float polarity{1.0f};
        float deadbandA{0.0f};
        float minimumReportableA{0.0f};
        float maximumAbsoluteCurrentA{0.0f};
        float minimumVoltageV{0.0f};
        float maximumVoltageV{26.0f};
        std::uint32_t readingIntervalMs{500};
    };

    class INA3221PowerSensor : public iotsmartsys::core::IPowerSensor
    {
    public:
        INA3221PowerSensor(INA3221Device &device, const INA3221PowerSensorConfig &config);

        void setup() override;
        void handle() override;
        long lastStateReadMillis() const override;
        const iotsmartsys::core::PowerMeasurement &powerMeasurement() const override;

    protected:
        virtual bool setupDevice();
        virtual bool deviceAvailable() const;
        virtual float readBusVoltage(std::uint8_t channel);
        virtual float readCurrentAmps(std::uint8_t channel);
        virtual std::uint32_t nowMillis() const;

    private:
        bool configValid() const;
        void invalidate(iotsmartsys::core::PowerMeasurementStatus status);

        INA3221Device &_device;
        INA3221PowerSensorConfig _config;
        iotsmartsys::core::PowerMeasurement _measurement;
        std::uint32_t _lastEvaluationMs{0};
        unsigned long _lastStateReadMs{0};
        bool _setupComplete{false};
        bool _evaluated{false};
    };
}
