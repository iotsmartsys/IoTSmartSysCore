#pragma once

#include "CapabilityConfig.h"
#include "Core/Models/DigitalLogic.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/ITemperatureSensor.h"
#include "Contracts/Sensors/IHumiditySensor.h"
#include "Contracts/Sensors/IGlpSensor.h"
#include "Contracts/Sensors/IGlpMeter.h"
#include "Contracts/Sensors/IColorSensor.h"

namespace iotsmartsys::app
{

    struct LightConfig : public CapabilityConfig
    {
        uint8_t pin;
        bool activeHigh = true;
        bool initialOn = false;
    };

    struct AlarmConfig : public CapabilityConfig
    {
        uint8_t pin;
        int activeState = 1; // 1 = HIGH, 0 = LOW
    };

    struct DoorSensorConfig : public CapabilityConfig
    {
        uint8_t pin;
    };

    struct PirSensorConfig : public CapabilityConfig
    {
        uint8_t pin;
        int toleranceTime = 5; // in seconds
    };

    struct SwitchPlugConfig : public CapabilityConfig
    {
        uint8_t pin;
        DigitalLogic switchLogic = DigitalLogic::NORMAL;
    };

    struct ClapSensorConfig : public CapabilityConfig
    {
        uint8_t pin;
        int toleranceTime = 5; // in seconds
    };

    struct PushButtonConfig : public CapabilityConfig
    {
        uint8_t pin;
        int toleranceTimeMs = 50; // debounce in ms
    };

    struct TouchButtonConfig : public CapabilityConfig
    {
        uint8_t pin;
        int toleranceTimeMs = 50; // debounce in ms
    };

    struct WaterFlowHallSensorConfig : public CapabilityConfig
    {
        uint8_t pin;
    };

    struct WaterLevelSensorConfig : public CapabilityConfig
    {
        // sensor instance provided by the caller; builder won't own the sensor
        iotsmartsys::core::IWaterLevelSensor *sensor{nullptr};
    };

    struct GlpSensorConfig : public CapabilityConfig
    {
        // sensor instance provided by the caller; builder won't own the sensor
        iotsmartsys::core::IGlpSensor *sensor{nullptr};
    };

    struct GlpMeterConfig : public CapabilityConfig
    {
        // sensor instance provided by the caller; builder won't own the sensor
        iotsmartsys::core::IGlpMeter *sensor{nullptr};
    };

    struct OperationalColorSensorConfig : public CapabilityConfig
    {
        // sensor instance provided by the caller; builder won't own the sensor
        iotsmartsys::core::IColorSensor *sensor{nullptr};
        // milliseconds between reads
        unsigned long readIntervalMs = 60000;
    };

    struct TemperatureSensorConfig : public CapabilityConfig
    {
        // sensor instance provided by the caller; builder won't own the sensor
        iotsmartsys::core::ITemperatureSensor *sensor{nullptr};
    };

    struct HumiditySensorConfig : public CapabilityConfig
    {
        // sensor instance provided by the caller; builder won't own the sensor
        iotsmartsys::core::IHumiditySensor *sensor{nullptr};
    };

} // namespace iotsmartsys::app