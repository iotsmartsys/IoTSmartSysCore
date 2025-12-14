
#pragma once

namespace iotsmartsys::core
{
    enum class ICapabilityType
    {
        None = 0,
        TemperatureSensor = 1,
        HumiditySensor = 2,
        PressureSensor = 3,
        LightSensor = 4,
        MotionSensor = 5,
        SmokeSensor = 6,
        GasSensor = 7,
        WaterSensor = 8,
        DoorSensor = 9,
        WindowSensor = 10,
        DoorLock = 11,
        WindowLock = 12,
        LightActuator = 13,
        FanActuator = 14,
        HeaterActuator = 15,
        CoolerActuator = 16,
        SprinklerActuator = 17,
        AlarmActuator = 18,
        Camera = 19,
        Speaker = 20,
        Microphone = 21,
        Display = 22,
        Motor = 23,
        Pump = 24,
        Valve = 25,
        Switch = 26,
        Relay = 27,
        Actuator = 28,
        Power = 29,
        RainSensor = 30,
        TIME_OF_DAY = 31,
        DoorbellPress = 32,
        setPowerState = 33,
        InformationFloat = 34,
        LEDActuator = 35,
        DistanceSensor = 36
    };

#define TOGGLE_COMMAND "toggle"
#define POWER_ON_COMMAND "on"
#define POWER_OFF_COMMAND "off"

#define AIR_CONDITIONER_TYPE "Air Conditioner"
#define AIR_HUMIDIFIER_TYPE "Air Humidifier"
#define ALARM_ON "on"
#define ALARM_OFF "off"
#define ALARM_ACTUATOR_TYPE "Alarm Actuator"
#define BATTERY_LEVEL_TYPE "Battery Level (%)"
#define LIGHT_SENSOR_TYPE "Light Sensor"

#define CLAP_DETECTED "detected"
#define CLAP_NO_DETECTED "undetected"
#define CLAP_SENSOR_TYPE "Clap Sensor"

#define DISTANCE_SENSOR_TYPE "Distance Sensor"
#define DOOR_SENSOR_OPEN "open"
#define DOOR_SENSOR_CLOSED "closed"
#define DOOR_SENSOR_TYPE "Door Sensor"
#define HEIGHT_WATER_LEVEL_SENSOR_TYPE "Distance Sensor"
#define HUMIDITY_SENSOR_TYPE "Humidity Sensor"
#define IR_TYPE "IR Command"
#define PROXIMITY_DETECTED "detected"
#define PROXIMITY_NO_DETECTED "undetected"
#define PROXIMITY_SENSOR_TYPE "Proximity Sensor"
#define LED_STATE_ON "on"
#define LED_STATE_OFF "off"
#define LED_ACTUATOR_TYPE "LED Actuator"
#define LIGHT_STATE_ON "on"
#define LIGHT_STATE_OFF "off"
#define LIGHT_ACTUATOR_TYPE "Light Actuator"
#define LIGHT_SENSOR_TYPE "Light Sensor"
#define OPERATIONAL_COLOR_SENSOR_NORMAL "Normal"
#define OPERATIONAL_COLOR_SENSOR_OK "Ok"
#define OPERATIONAL_COLOR_SENSOR_FAIL "Error"
#define OPERATIONAL_COLOR_SENSOR_TYPE "Operational Color Sensor"
#define PIR_DETECTED "detected"
#define PIR_NO_DETECTED "undetected"
#define PIR_SENSOR_TYPE "Motion Sensor"

#define LIGHT_STATE_ON "on"
#define LIGHT_STATE_OFF "off"
#define LIGHT_ACTUATOR_TYPE "Light Actuator"

#define SWITCH_STATE_ON POWER_ON_COMMAND
#define SWITCH_STATE_OFF POWER_OFF_COMMAND
#define SWITCH_TYPE "Switch"
#define SWITCH_PLUG_TYPE "Switch Plug"

#define TEMPERATURE_SENSOR_TYPE "Temperature Sensor"

#define BUTTON_PRESSED "pressed"
#define BUTTON_NO_PRESSED "undetected"
#define BUTTON_TOUCH_TYPE "Button Touch"

#define VALVE_STATE_OPEN "open"
#define VALVE_STATE_CLOSED "closed"
#define VaLVE_ACTUATOR_TYPE "Valve Actuator"

#define WATER_FLOW_SENSOR_TYPE "Water Flow (L)"
#define WATER_LEVEL_LITERS_SENSOR_TYPE "Water Level (L)"
#define WATER_LEVEL_PERCENT_SENSOR_TYPE "Water Level (%)"

#define PUSH_BUTTON_TYPE "Push Button"
#define PUSH_BUTTON_NO_PRESSED "no_pressed"
#define PUSH_BUTTON_PRESSED_ "pressed_"
#define PUSH_BUTTON_PRESSED_1_X BUTTON_PRESSED_ "x1"
#define PUSH_BUTTON_PRESSED_2_X BUTTON_PRESSED_ "x2"
#define PUSH_BUTTON_PRESSED_3_X BUTTON_PRESSED_ "x3"
#define PUSH_BUTTON_PRESSED_4_X BUTTON_PRESSED_ "x4"

#define GLP_METER_TYPE "GLP Meter"
#define GLP_SENSOR_TYPE "GLP Sensor Level"
#define GLP_SENSOR_LEVEL_LOW "low"
#define GLP_SENSOR_LEVEL_MEDIUM "medium"
#define GLP_SENSOR_LEVEL_HIGH "high"
#define GLP_SENSOR_LEVEL_NONE "none"

#define DEFAULT_TIME_BETWEEN_UPDATES 600000

} // namespace iotsmartsys::core
