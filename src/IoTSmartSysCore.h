#pragma once
#include <Arduino.h>
#include "Infra/Transports/IMessageClient.h"

#include "Infra/Wifi/WifiHelper.h"
#include <vector>
#include <functional>
#include <string>
#include "Platform/Builders/CapabilityBuilder.h"
#include "Core/Models/DeviceAnnouncement.h"
#include "Core/Models/PropertyState.h"
#include "Capabilities/GlpSensorCapability.h"

#ifdef ESP_NOW_ENABLED
#include "esp_now_utils/esp_now_utils.h"
#endif
#include "Infra/Settings/Models/Settings.h"
#include "Core/Models/Property.h"


#ifndef DISCOVERY_TOPIC_DEFAULT
#define DISCOVERY_TOPIC_DEFAULT "smarthome/discovery"
#endif

class IoTCore
{
public:
    IoTCore();
    IoTCore(const char *device_name, std::vector<Capability *> capabilities);

    ~IoTCore();

    void setup();
    void handle();
    void subscribe(const char *topic);
    void sendState(CapabilityState state);
    void sendDeviceIncoming(DeviceAnnouncement announcement);
    bool isOk();
    LEDCapability &configureLEDControl(int ledPin, DigitalLogic ledLogic = DigitalLogic::NORMAL);
    void setBuild(const String &build);

public:
    IMessageClient *transport;
    CollectionCapabilityBuilder *capabilityBuilder = new CollectionCapabilityBuilder();
    const Settings *settings;

private:
#if defined(AUTO_UPDATE_ENABLED)
#endif
    std::vector<Capability *> capabilities;
    std::vector<Property *> properties;
    String discoveryTopic;
    String device_name;
    int lastSendDeviceState = 0;
    int lastRSSI = 0;

    LEDCapability *ledCapability = nullptr;
    void capabilitiesHandle();
    void notifyDeviceState();
    void addDefaultProperties();
    void resolveDeviceIdentity();
    void resolveDiscoveryTopic();

#ifdef ESP_NOW_ENABLED

    void setupEspNow();
#endif

public:
    Capability &addCapability(Capability *capability);
    Property &addProperty(const String &name, const String &value);
#ifdef RGB_ENABLED
    RGBLightCapability &addRGBLightCapability(int pin, int red, int green, int blue);
#endif

#ifdef OPERATIONAL_COLOR_SENSOR_ENABLED
    OperationalColorSensorCapability &addOperationalColorSensorCapability(unsigned int readIntervalMs = 1);
#endif

#ifdef IRREMOTE_ENABLED
    AirHumidifierCapability &addAirHumidifierCapability(int irPin);
    AirConditionerCapability &addAirConditionerCapability(int irPin);
#endif

#ifdef GLP_METER_ENABLED
    GlpMeterCapability &addGlpMeterCapability(String capability_name, int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg);
    GlpMeterCapability &addGlpMeterCapability(int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg);
#endif
    ClapSensorCapability &addClapSensorCapability(String capability_name, int clapPin, int toleranceTime = 1);
    SwitchCapability &addSwitchCapability(String capability_name, int switchPin, DigitalLogic switchLogic = DigitalLogic::NORMAL);
    SwitchCapability &addSwitchCapability(int switchPin, DigitalLogic switchLogic = DigitalLogic::NORMAL);
    SwitchPlugCapability &addSwitchPlugCapability(String capability_name, int switchPin, DigitalLogic switchLogic = DigitalLogic::NORMAL);
    SwitchPlugCapability &addSwitchPlugCapability(int switchPin, DigitalLogic switchLogic = DigitalLogic::NORMAL);
    IRProximitySensorCapability &addIRProximitySensorCapability(String capability_name, int irPin);
    /// @brief Add a PIR sensor capability
    /// @param pirPin The GPIO pin the PIR sensor is connected to
    /// @param toleranceTime The time (in minutes) to wait before triggering the sensor again
    /// @return A reference to the created PirSensorCapability
    PirSensorCapability &addPirSensorCapability(int pirPin, int toleranceTime = 5);
    /// @brief Add a PIR sensor capability with a custom name
    /// @param capability_name The name of the capability
    /// @param pirPin The GPIO pin the PIR sensor is connected to
    /// @param toleranceTime The time (in minutes) to wait before triggering the sensor again
    /// @return A reference to the created PirSensorCapability
    PirSensorCapability &addPirSensorCapability(String capability_name, int pirPin, int toleranceTime = 5);
    LightCapability &addLightCapability(String capability_name, int lightPin);
    LightCapability &addLightCapability(int lightPin, DigitalLogic digitalLogic = DigitalLogic::NORMAL);
    AlarmCapability &addAlarmCapability(int alarmPin, DigitalLogic stateLogic = DigitalLogic::INVERSE);
    AlarmCapability &addAlarmCapability(int alarmPin, long ringDuration, DigitalLogic stateLogic = DigitalLogic::NORMAL);
    AlarmCapability &addAlarmCapability(String capability_name, int alarmPin, DigitalLogic stateLogic = DigitalLogic::NORMAL);
    DoorSensorCapability &addDoorSensorCapability(int doorPin);
    LEDCapability &addLEDCapability(String capability_name, int ledPin, DigitalLogic ledLogic = DigitalLogic::NORMAL);
    ValveCapability &addValveCapability(String capability_name, int valvePin, DigitalLogic valveLogic = DigitalLogic::NORMAL);

#ifdef BH1750_ENABLED
    BH1750SensorCapability &addLuminositySensorCapability(int factor, float readInterval = 1);
#endif

    DistanceSensorCapability &addHC_SR04DistanceSensorCapability(int trigPin, int echoPin);
    DistanceSensorCapability &addHC_SR04DistanceSensorCapability(int trigPin, int echoPin, long minDistance, long maxDistance);

#ifdef DHT_ENABLED
    HumiditySensorCapability &addHumiditySensorCapability(DHT *dht, int intervalMinutes = 10);
    TemperatureSensorCapability &addTemperatureSensorCapability(DHT *dht, int intervalMinutes = 10);
#endif

#ifdef DS18B20_ENABLED
    TemperatureSensorCapability &addTemperatureSensorCapability(int pin, int intervalMinutes = 10);
    TemperatureSensorCapability &addTemperatureSensorCapability(String capability_name, int pin, int intervalMinutes);
#endif

    WaterLevelPercentCapability &addWaterLevelPercentageCapability(String capability_name, WaterLevelSensor *sensor);
    WaterLevelPercentCapability &addWaterLevelPercentageCapability(WaterLevelSensor *sensor);
    WaterLevelLitersCapability &addWaterLevelLitersCapability(String capability_name, WaterLevelSensor *sensor);
    WaterLevelLitersCapability &addWaterLevelLitersCapability(WaterLevelSensor *sensor);
    HeightWaterLevelCapability &addWaterHeightCapability(String capability_name, WaterLevelSensor *sensor);
    HeightWaterLevelCapability &addWaterHeightCapability(WaterLevelSensor *sensor);
    WaterFlowHallSensorCapability &addWaterFlowHallSensorCapability(String capability_name, int pin);
    WaterFlowHallSensorCapability &addWaterFlowHallSensorCapability(String capability_name, int pin, ValveCapability *valve);
};
