#pragma once

#include "Contracts/Sensors/ISensorFactory.h"
#include "Platform/Arduino/Sensors/DHTSensor.h"
#include "Platform/Arduino/Sensors/DHTSensor.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::infra::factories
{
    using namespace iotsmartsys::core;

    class SensorFactory : public core::ISensorFactory
    {
    public:
        SensorFactory(ILogger &logger);
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

        /// @brief Creates a water level sensor.
        /// @param triggerPin The GPIO pin for the trigger.
        /// @param echoPin The GPIO pin for the echo.
        /// @param minDistance The minimum distance for the sensor.
        /// @param maxDistance The maximum distance for the sensor.
        /// @param recipentType The type of recipient for the water level sensor.
        /// @return A unique pointer to the created water level sensor.
        std::unique_ptr<iotsmartsys::core::IWaterLevelSensor> createWaterLevelSensor(const int triggerPin, const int echoPin, long minDistance, long maxDistance, core::WaterLevelRecipentType recipentType) override;

        /// @brief Creates a GLP sensor.
        /// @param pinAO The analog output pin.
        /// @param pinDO The digital output pin.
        /// @return A unique pointer to the created GLP sensor.
        std::unique_ptr<core::IGlpSensor> createGlpSensor(int pinAO, int pinDO) override;

        /// @brief Creates a GLP meter.
        /// @param pinAO The analog output pin.
        /// @return A unique pointer to the created GLP meter.
        std::unique_ptr<core::IGlpMeter> createGlpMeter(int pinAO) override;

    private:
        ILogger &_logger;
    };

} // namespace iotsmartsys::infra::factories