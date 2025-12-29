#pragma once

#include "Contracts/Sensors/ISensorFactory.h"
#include "Platform/Arduino/Sensors/DHTSensor.h"
#include "Platform/Arduino/Sensors/DHTSensor.h"

namespace iotsmartsys::infra::factories
{
    class SensorFactory : public core::ISensorFactory
    {
    public:
        ~SensorFactory() override = default;

        /// @brief Creates a temperature sensor.
        /// @param gpio The GPIO pin to which the sensor is connected.
        /// @param model The model of the temperature sensor.
        /// @return A unique pointer to the created temperature sensor.
        std::unique_ptr<core::ITemperatureSensor> createTemperatureSensor(const int gpio, core::TemperatureSensorModel model) override;

        /// @brief Creates a DHT sensor.
        /// @param gpio The GPIO pin to which the sensor is connected.
        /// @return A unique pointer to the created DHT sensor.
        std::unique_ptr<iotsmartsys::platform::arduino::DHTSensor> createDHTSensor(const int gpio);

        /// @brief Creates a luminosity sensor.
        /// @return A unique pointer to the created luminosity sensor.
        std::unique_ptr<iotsmartsys::core::ILuminositySensor> createLuminositySensor(const int gpioSDA, const int gpioSCL) override;

    private:
    };

} // namespace iotsmartsys::infra::factories