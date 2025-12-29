#pragma once

#include "Contracts/Sensors/ISensorFactory.h"
#include "Platform/Arduino/Sensors/DHTSensor.h"

namespace iotsmartsys::infra::factories
{
    class SensorFactory : public core::ISensorFactory
    {
    public:
        ~SensorFactory() override = default;

        std::unique_ptr<core::ITemperatureSensor> createTemperatureSensor(const int gpio, core::TemperatureSensorModel model) override;

        std::unique_ptr<iotsmartsys::platform::arduino::DHTSensor> createDHTSensor(const int gpio);

    private:
    };

} // namespace iotsmartsys::infra::factories