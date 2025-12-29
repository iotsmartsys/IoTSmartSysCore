#pragma once

#include <memory>
#include "Contracts/Sensors/ITemperatureSensor.h"
#include "Contracts/Sensors/IGlpSensor.h"
#include "Contracts/Sensors/SensorModel.h"

namespace iotsmartsys::core
{
    class ISensorFactory
    {
    public:
        virtual ~ISensorFactory() = default;

        virtual std::unique_ptr<ITemperatureSensor> createTemperatureSensor(const int gpio, TemperatureSensorModel model) = 0;
    };

} // namespace iotsmartsys::core