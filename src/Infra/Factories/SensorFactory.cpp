#include "Infra/Factories/SensorFactory.h"

#if IOTSMARTSYS_SENSORS_ENABLED
#include "Platform/Arduino/Sensors/DS18B20TemperatureSensor.h"
#include "Platform/Arduino/Sensors/Bh1750LuminositySensor.h"
#include "Platform/Arduino/Sensors/ArduinoUltrassonicWaterLevelSensor.h"
#include "Platform/Arduino/Sensors/ArduinoGlpSensor.h"
#include "Platform/Arduino/Sensors/HX711WeightMeter.h"
#endif
#include <memory>

namespace iotsmartsys::infra::factories
{
    using namespace iotsmartsys::core;
#if IOTSMARTSYS_SENSORS_ENABLED
    using namespace iotsmartsys::platform::arduino;
#ifdef DHT_SENSOR_ENABLED
    using iotsmartsys::platform::arduino::DHTSensor;
#endif
#ifdef DS18B20_SENSOR_ENABLED
    using iotsmartsys::platform::arduino::DS18B20TemperatureSensor;
#endif
#endif

    SensorFactory::SensorFactory(ILogger &logger) : _logger(logger)
    {
    }

    std::unique_ptr<core::ITemperatureSensor> SensorFactory::createTemperatureSensor(const int gpio, core::TemperatureSensorModel model)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        switch (model)
        {
#ifdef DHT_SENSOR_ENABLED
        case core::TemperatureSensorModel::DHT:
            return createDHTSensor(gpio);
#endif
#ifdef DS18B20_SENSOR_ENABLED
        case core::TemperatureSensorModel::DS18B20:
            return std::unique_ptr<core::ITemperatureSensor>(new DS18B20TemperatureSensor(gpio));
#endif
        default:
            return nullptr;
        }
#else
        (void)gpio;
        (void)model;
        return nullptr;
#endif
    }

#if IOTSMARTSYS_SENSORS_ENABLED && defined(DHT_SENSOR_ENABLED)
    std::unique_ptr<iotsmartsys::platform::arduino::DHTSensor> SensorFactory::createDHTSensor(const int gpio)
    {
        return std::unique_ptr<iotsmartsys::platform::arduino::DHTSensor>(new iotsmartsys::platform::arduino::DHTSensor(gpio));
    }
#endif

    std::unique_ptr<iotsmartsys::core::ILuminositySensor> SensorFactory::createLuminositySensor(const int gpioSDA, const int gpioSCL)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        return std::unique_ptr<Bh1750LuminositySensor>(new Bh1750LuminositySensor(gpioSDA, gpioSCL));
#else
        (void)gpioSDA;
        (void)gpioSCL;
        return nullptr;
#endif
    }

    std::unique_ptr<iotsmartsys::core::IWaterLevelSensor> SensorFactory::createWaterLevelSensor(const int triggerPin, const int echoPin, long minDistance, long maxDistance, WaterLevelRecipentType recipentType)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        return std::unique_ptr<ArduinoUltrassonicWaterLevelSensor>(new ArduinoUltrassonicWaterLevelSensor(new SensorUltrassonicHCSR04(triggerPin, echoPin, minDistance, maxDistance), recipentType));
#else
        (void)triggerPin;
        (void)echoPin;
        (void)minDistance;
        (void)maxDistance;
        (void)recipentType;
        return nullptr;
#endif
    }

    std::unique_ptr<core::IGlpSensor> SensorFactory::createGlpSensor(int pinAO, int pinDO)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        return std::unique_ptr<ArduinoGlpSensor>(new ArduinoGlpSensor(pinAO, pinDO, _logger));
#else
        (void)pinAO;
        (void)pinDO;
        return nullptr;
#endif
    }

    std::unique_ptr<core::IGlpMeter> SensorFactory::createGlpMeter(int DOutPin, int SCKPin)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        HX711WeightMeter::Config cfg;
        cfg.doutPin = DOutPin;
        cfg.sckPin = SCKPin;

        return std::unique_ptr<HX711WeightMeter>(new HX711WeightMeter(cfg));
#else
        (void)DOutPin;
        (void)SCKPin;
        return nullptr;
#endif
    }

    std::unique_ptr<core::IGlpMeter> SensorFactory::createGlpMeter(int DOutPin, int SCKPin, float tare)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        HX711WeightMeter::Config cfg;
        cfg.doutPin = DOutPin;
        cfg.sckPin = SCKPin;
        cfg.tare = tare;

        return std::unique_ptr<HX711WeightMeter>(new HX711WeightMeter(cfg));
#else
        (void)DOutPin;
        (void)SCKPin;
        (void)tare;
        return nullptr;
#endif
    }

    std::unique_ptr<core::IGlpMeter> SensorFactory::createGlpMeter(int DOutPin, int SCKPin, float tare, float variationTolerance)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        HX711WeightMeter::Config cfg;
        cfg.doutPin = DOutPin;
        cfg.sckPin = SCKPin;
        cfg.tare = tare;
        cfg.variationTolerance = variationTolerance;

        return std::unique_ptr<HX711WeightMeter>(new HX711WeightMeter(cfg));
#else
        (void)DOutPin;
        (void)SCKPin;
        (void)tare;
        return nullptr;
#endif
    }

} // namespace iotsmartsys::infra::factories
