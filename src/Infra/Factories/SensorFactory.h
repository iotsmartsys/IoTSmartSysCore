#pragma once

#include "Contracts/Sensors/ISensorFactory.h"

namespace iotsmartsys::infra::factories
{
    class SensorFactory : public core::ISensorFactory
    {
    public:
        ~SensorFactory() override = default;

        std::unique_ptr<core::ITemperatureSensor> createTemperatureSensor(const int gpio, core::TemperatureSensorModel model) override;

    private:
    };

} // namespace iotsmartsys::infra::factories