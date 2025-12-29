#include "Infra/Factories/SensorFactory.h"
#include "Platform/Arduino/Sensors/DHTTemperatureSensor.h"
#include "Platform/Arduino/Sensors/DS18B20TemperatureSensor.h"
#include <memory>

namespace iotsmartsys::infra::factories
{
    using iotsmartsys::platform::arduino::DHTTemperatureSensor;
    using iotsmartsys::platform::arduino::DS18B20TemperatureSensor;

    std::unique_ptr<core::ITemperatureSensor> SensorFactory::createTemperatureSensor(const int gpio, core::TemperatureSensorModel model)
    {
        switch (model)
        {
        case core::TemperatureSensorModel::DHT:
            return std::unique_ptr<core::ITemperatureSensor>(new DHTTemperatureSensor(gpio));
        case core::TemperatureSensorModel::DS18B20:
            return std::unique_ptr<core::ITemperatureSensor>(new DS18B20TemperatureSensor(gpio));
        default:
            return nullptr;
        }
    }

} // namespace iotsmartsys::infra::factories
