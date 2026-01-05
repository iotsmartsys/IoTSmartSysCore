#pragma once

#include <memory>
#include "Contracts/Sensors/ITemperatureSensor.h"
#include "Contracts/Sensors/IGlpSensor.h"
#include "Contracts/Sensors/SensorModel.h"
#include "Contracts/Sensors/ILuminositySensor.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/WaterLevelRecipentType.h"
#include "Contracts/Sensors/IGlpMeter.h"

namespace iotsmartsys::core
{
    class ISensorFactory
    {
    public:
        virtual ~ISensorFactory() = default;

        virtual std::unique_ptr<ITemperatureSensor> createTemperatureSensor(const int gpio, TemperatureSensorModel model) = 0;

        virtual std::unique_ptr<ILuminositySensor> createLuminositySensor(const int gpioSDA, const int gpioSCL) = 0;
        virtual std::unique_ptr<IWaterLevelSensor> createWaterLevelSensor(const int triggerPin, const int echoPin, long minDistance, long maxDistance, WaterLevelRecipentType recipentType) = 0;
        virtual std::unique_ptr<IGlpSensor> createGlpSensor(int pinAO, int pinDO) = 0;
        virtual std::unique_ptr<IGlpMeter> createGlpMeter(int pinAO) = 0;
    };

} // namespace iotsmartsys::core