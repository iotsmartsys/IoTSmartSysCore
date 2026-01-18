#pragma once

#include "HardwareConfig.h"
#include "Core/Models/DigitalLogic.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/ITemperatureSensor.h"
#include "Contracts/Sensors/IHumiditySensor.h"
#include "Contracts/Sensors/IGlpSensor.h"
#include "Contracts/Sensors/IGlpMeter.h"
#include "Contracts/Sensors/IColorSensor.h"
#include "Contracts/Sensors/ILuminositySensor.h"

namespace iotsmartsys::app
{

    class LightConfig : public HardwareConfig
    {
    public:
        using HardwareConfig::HardwareConfig;
    };

    class AlarmConfig : public HardwareConfig
    {
    public:
        using HardwareConfig::HardwareConfig;
    };

    class DoorSensorConfig : public HardwareConfig
    {
    public:
        using HardwareConfig::HardwareConfig;

        bool highIsOn = false;

    };

    class PirSensorConfig : public InputHardwareConfig
    {
    public:
        using InputHardwareConfig::InputHardwareConfig;
    };

    class SwitchConfig : public HardwareConfig
    {
    public:
        using HardwareConfig::HardwareConfig;
    };
    class ValveConfig : public HardwareConfig
    {
    public:
        using HardwareConfig::HardwareConfig;
    };

    class ClapSensorConfig : public InputHardwareConfig
    {
    public:
        using InputHardwareConfig::InputHardwareConfig;
    };

    class PushButtonConfig : public InputHardwareConfig
    {
    public:
        using InputHardwareConfig::InputHardwareConfig;
        bool highIsOn = false;
    };

    class TouchButtonConfig : public InputHardwareConfig
    {
    public:
        using InputHardwareConfig::InputHardwareConfig;
    };

    class WaterFlowHallSensorConfig : public InputHardwareConfig
    {
    public:
        using InputHardwareConfig::InputHardwareConfig;
    };

    class WaterLevelSensorConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::IWaterLevelSensor *sensor{nullptr};
    };

    class GlpSensorConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::IGlpSensor *sensor{nullptr};
    };

    class GlpMeterConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::IGlpMeter *sensor{nullptr};
    };

    class OperationalColorSensorConfig : public InputHardwareConfig
    {
    public:
        iotsmartsys::core::IColorSensor *sensor{nullptr};
    };

    class TemperatureSensorConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::ITemperatureSensor *sensor{nullptr};
        long readIntervalMs = 60000;
    };

    class HumiditySensorConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::IHumiditySensor *sensor{nullptr};
        long readIntervalMs = 60000;
    };

    class LuminositySensorConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::ILuminositySensor *sensor{nullptr};
        long readIntervalMs = 60000;
        float variationTolerance = 2.0f; // in lux
    };
} // namespace iotsmartsys::app