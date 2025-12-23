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

    class LightConfig : public HardwareConfig
    {
    };

    class AlarmConfig : public HardwareConfig
    {
    };

    class DoorSensorConfig : public HardwareConfig
    {
    };

    class PirSensorConfig : public InputHardwareConfig
    {
    };

    class SwitchPlugConfig : public HardwareConfig
    {
    };

    class ClapSensorConfig : public InputHardwareConfig
    {
    };

    class PushButtonConfig : public InputHardwareConfig
    {
    };

    class TouchButtonConfig : public InputHardwareConfig
    {
    };

    class WaterFlowHallSensorConfig : public InputHardwareConfig
    {
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
    };

    class HumiditySensorConfig : public HardwareConfig
    {
    public:
        iotsmartsys::core::IHumiditySensor *sensor{nullptr};
    };

} // namespace iotsmartsys::app