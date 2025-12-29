#include "Infra/Factories/SensorFactory.h"
#include "Platform/Arduino/Sensors/DS18B20TemperatureSensor.h"
#include "Platform/Arduino/Sensors/Bh1750LuminositySensor.h"
#include <memory>

namespace iotsmartsys::infra::factories
{
    using namespace iotsmartsys::platform::arduino;
    using iotsmartsys::platform::arduino::DHTSensor;
    using iotsmartsys::platform::arduino::DS18B20TemperatureSensor;

    std::unique_ptr<core::ITemperatureSensor> SensorFactory::createTemperatureSensor(const int gpio, core::TemperatureSensorModel model)
    {
        switch (model)
        {
        case core::TemperatureSensorModel::DHT:
            return createDHTSensor(gpio);
        case core::TemperatureSensorModel::DS18B20:
            return std::unique_ptr<core::ITemperatureSensor>(new DS18B20TemperatureSensor(gpio));
        default:
            return nullptr;
        }
    }

    std::unique_ptr<iotsmartsys::platform::arduino::DHTSensor> SensorFactory::createDHTSensor(const int gpio)
    {
        return std::unique_ptr<iotsmartsys::platform::arduino::DHTSensor>(new iotsmartsys::platform::arduino::DHTSensor(gpio));
    }

    std::unique_ptr<iotsmartsys::core::ILuminositySensor> SensorFactory::createLuminositySensor(const int gpioSDA, const int gpioSCL)
    {
        return std::unique_ptr<Bh1750LuminositySensor>(new Bh1750LuminositySensor(gpioSDA, gpioSCL));
    }

} // namespace iotsmartsys::infra::factories
