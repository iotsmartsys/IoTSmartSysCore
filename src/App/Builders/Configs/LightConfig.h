#pragma once

#include "CapabilityConfig.h"
#include "Core/Models/DigitalLogic.h"

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

} // namespace iotsmartsys::app