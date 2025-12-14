#pragma once

#include <Arduino.h>
#include <vector>
#include "Capabilities/Capability.h"
#include "Capabilities/PirSensorCapability.h"
#include "Capabilities/DoorSensorCapability.h"
#include "Capabilities/LEDCapability.h"
#include "Capabilities/DistanceSensorCapability.h"
#ifdef BH1750_ENABLED
#include "Capabilities/BH1750SensorCapability.h"
#endif

#include "Capabilities/HumiditySensorCapability.h"
#include "Capabilities/TemperatureSensorCapability.h"

#include "Capabilities/ValveCapability.h"
#include "Infra/Sensors/Modules/WaterLevelSensor.h"
#include "Capabilities/WaterLevelPercentageCapability.h"
#include "Capabilities/WaterLevelLitersCapability.h"
#include "Capabilities/HeightWaterLevelCapability.h"
#include "Capabilities/WaterFlowHallSensorCapability.h"
#include "Capabilities/SwitchCapability.h"
#include "Capabilities/IRProximitySensorCapability.h"
#include "Capabilities/BatteryLevelCapability.h"
#include "Capabilities/ClapSensorCapability.h"
#include "Capabilities/SwitchPlugCapability.h"
#include "Capabilities/OperationalColorSensorCapability.h"
#include "Capabilities/LEDCapability.h"

#ifdef GLP_METER_ENABLED
#include "Capabilities/GlpMeterCapability.h"
#endif

#ifdef RGB_ENABLED
#include "Capabilities/RGBLightCapability.h"
#endif
#include "Capabilities/AirHumidifierCapability.h"

#ifdef IRREMOTE_ENABLED
#include "Capabilities/AirConditionerCapability.h"
#endif

class CollectionCapabilityBuilder
{
public:
    CollectionCapabilityBuilder();

    Capability &addCapability(Capability *capability);
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
    PirSensorCapability &addPirSensorCapability(int pirPin, int toleranceTime = 5);
    PirSensorCapability &addPirSensorCapability(String capability_name, int pirPin, int toleranceTime = 5);
    DoorSensorCapability &addDoorSensorCapability(int doorPin);
    LEDCapability &addLEDCapability(String capability_name, int ledPin, DigitalLogic ledLogic = DigitalLogic::NORMAL);
    ValveCapability &addValveCapability(String capability_name, int valvePin, DigitalLogic valveLogic = DigitalLogic::NORMAL);

#ifdef BH1750_ENABLED
    BH1750SensorCapability &addLuminositySensorCapability(BH1750 *bh1750Sensor, int factor);
    BH1750SensorCapability &addLuminositySensorCapability(BH1750 *bh1750Sensor, int factor, float readInterval = 10);

    /// @brief Adiciona uma capability de sensor de luminosidade BH1750 ao IoTCore.
    /// @param factor Fator de variação de luminosidade para atualizar o valor (em lux).
    /// @param readInterval Intervalo de leitura em segundos. Padrão é 10 segundo.
    /// @return
    BH1750SensorCapability &addLuminositySensorCapability(int factor, float readInterval = 10);
#endif

    DistanceSensorCapability &addHC_SR04DistanceSensorCapability(int trigPin, int echoPin);
    DistanceSensorCapability &addHC_SR04DistanceSensorCapability(int trigPin, int echoPin, long minDistance, long maxDistance);

#ifdef DHT_ENABLED
    HumiditySensorCapability &addHumiditySensorCapability(DHT *dht, int intervalMinutes = 10);
    TemperatureSensorCapability &addTemperatureSensorCapability(DHT *dht, int intervalMinutes = 10);
#endif

#ifdef DS18B20_ENABLED
    TemperatureSensorCapability &addTemperatureSensorCapability(int pin, int intervalMinutes = 10);
#endif

    WaterLevelPercentCapability &addWaterLevelPercentageCapability(String capability_name, WaterLevelSensor *sensor);
    WaterLevelPercentCapability &addWaterLevelPercentageCapability(WaterLevelSensor *sensor);
    WaterLevelLitersCapability &addWaterLevelLitersCapability(String capability_name, WaterLevelSensor *sensor);
    WaterLevelLitersCapability &addWaterLevelLitersCapability(WaterLevelSensor *sensor);
    HeightWaterLevelCapability &addWaterHeightCapability(String capability_name, WaterLevelSensor *sensor);
    HeightWaterLevelCapability &addWaterHeightCapability(WaterLevelSensor *sensor);
    WaterFlowHallSensorCapability &addWaterFlowHallSensorCapability(String capability_name, int pin);
    WaterFlowHallSensorCapability &addWaterFlowHallSensorCapability(String capability_name, int pin, ValveCapability *valve);

    std::vector<Capability *> build();
    bool hasBuiltCapabilities();
    bool canBuild();

private:
    std::vector<Capability *> capabilities = {};
    bool hasFree = false;
};