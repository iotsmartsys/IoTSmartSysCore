#include "CapabilityBuilder.h"

CollectionCapabilityBuilder::CollectionCapabilityBuilder() {}

Capability &CollectionCapabilityBuilder::addCapability(Capability *capability)
{
    capabilities.push_back(capability);
    return *capability;
}

#ifdef GLP_METER_ENABLED
HX711 *scale = new HX711();
GlpMeterCapability &CollectionCapabilityBuilder::addGlpMeterCapability(String capability_name, int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg)
{
    GlpMeterCapability *glpMeterCapability = new GlpMeterCapability(capability_name, scale, dout_pin, sck_pin, tare_weight_kg, weight_capacity_kg);
    capabilities.push_back(glpMeterCapability);
    return *glpMeterCapability;
}

GlpMeterCapability &CollectionCapabilityBuilder::addGlpMeterCapability(int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg)
{
    return addGlpMeterCapability("", dout_pin, sck_pin, tare_weight_kg, weight_capacity_kg);
}
#endif

#ifdef RGB_ENABLED
RGBLightCapability &CollectionCapabilityBuilder::addRGBLightCapability(int pin, int red, int green, int blue)
{
    RGBLightCapability *rgbLightCapability = new RGBLightCapability(pin);
    rgbLightCapability->setColor(red, green, blue);
    capabilities.push_back(rgbLightCapability);
    return *rgbLightCapability;
}
#endif

#ifdef OPERATIONAL_COLOR_SENSOR_ENABLED
OperationalColorSensorCapability &CollectionCapabilityBuilder::addOperationalColorSensorCapability(unsigned int readIntervalMs)
{
    OperationalColorSensorCapability *operationalColorSensorCapability = new OperationalColorSensorCapability(readIntervalMs);
    capabilities.push_back(operationalColorSensorCapability);
    return *operationalColorSensorCapability;
}
#endif

#ifdef IRREMOTE_ENABLED
AirHumidifierCapability &CollectionCapabilityBuilder::addAirHumidifierCapability(int irPin)
{
    AirHumidifierCapability *airHumidifierCapability = new AirHumidifierCapability(irPin);
    capabilities.push_back(airHumidifierCapability);
    return *airHumidifierCapability;
}

AirConditionerCapability &CollectionCapabilityBuilder::addAirConditionerCapability(int irPin)
{
    AirConditionerCapability *airConditionerCapability = new AirConditionerCapability(irPin);
    capabilities.push_back(airConditionerCapability);
    return *airConditionerCapability;
}
#endif

ClapSensorCapability &CollectionCapabilityBuilder::addClapSensorCapability(String capability_name, int clapPin, int toleranceTime)
{
    ClapSensorCapability *clapSensorCapability = new ClapSensorCapability(capability_name, clapPin, toleranceTime);
    capabilities.push_back(clapSensorCapability);
    return *clapSensorCapability;
}

SwitchCapability &CollectionCapabilityBuilder::addSwitchCapability(String capability_name, int switchPin, DigitalLogic switchLogic)
{
    SwitchCapability *switchCapability = new SwitchCapability(capability_name, switchPin, switchLogic);
    capabilities.push_back(switchCapability);
    return *switchCapability;
}

SwitchCapability &CollectionCapabilityBuilder::addSwitchCapability(int switchPin, DigitalLogic switchLogic)
{
    SwitchCapability *switchCapability = new SwitchCapability("", switchPin, switchLogic);
    capabilities.push_back(switchCapability);
    return *switchCapability;
}

SwitchPlugCapability &CollectionCapabilityBuilder::addSwitchPlugCapability(String capability_name, int switchPin, DigitalLogic switchLogic)
{
    SwitchPlugCapability *switchPlugCapability = new SwitchPlugCapability(capability_name, switchPin, switchLogic);
    capabilities.push_back(switchPlugCapability);
    return *switchPlugCapability;
}
SwitchPlugCapability &CollectionCapabilityBuilder::addSwitchPlugCapability(int switchPin, DigitalLogic switchLogic)
{
    SwitchPlugCapability *switchPlugCapability = new SwitchPlugCapability("", switchPin, switchLogic);
    capabilities.push_back(switchPlugCapability);
    return *switchPlugCapability;
}

IRProximitySensorCapability &CollectionCapabilityBuilder::addIRProximitySensorCapability(String capability_name, int irPin)
{
    IRProximitySensorCapability *irProximitySensorCapability = new IRProximitySensorCapability(capability_name, irPin);
    capabilities.push_back(irProximitySensorCapability);
    return *irProximitySensorCapability;
}

PirSensorCapability &CollectionCapabilityBuilder::addPirSensorCapability(int pirPin, int toleranceTime)
{
    PirSensorCapability *pirSensorCapability = new PirSensorCapability(pirPin, toleranceTime);
    capabilities.push_back(pirSensorCapability);
    return *pirSensorCapability;
}

PirSensorCapability &CollectionCapabilityBuilder::addPirSensorCapability(String capability_name, int pirPin, int toleranceTime)
{
    PirSensorCapability *pirSensorCapability = new PirSensorCapability(pirPin, toleranceTime);
    pirSensorCapability->applyRenamedName(capability_name);
    capabilities.push_back(pirSensorCapability);
    return *pirSensorCapability;
}

DoorSensorCapability &CollectionCapabilityBuilder::addDoorSensorCapability(int doorPin)
{
    DoorSensorCapability *doorSensorCapability = new DoorSensorCapability(doorPin);
    capabilities.push_back(doorSensorCapability);
    return *doorSensorCapability;
}

LEDCapability &CollectionCapabilityBuilder::addLEDCapability(String capability_name, int ledPin, DigitalLogic ledLogic)
{
    LEDCapability *ledCapability = new LEDCapability(capability_name, ledPin, ledLogic);
    capabilities.push_back(ledCapability);
    return *ledCapability;
}

ValveCapability &CollectionCapabilityBuilder::addValveCapability(String capability_name, int valvePin, DigitalLogic valveLogic)
{
    ValveCapability *valveCapability = new ValveCapability(capability_name, valvePin, valveLogic);
    capabilities.push_back(valveCapability);
    return *valveCapability;
}

#ifdef BH1750_ENABLED
BH1750SensorCapability &CollectionCapabilityBuilder::addLuminositySensorCapability(BH1750 *bh1750Sensor, int factor)
{
    BH1750SensorCapability *luminositySensorCapability = new BH1750SensorCapability(bh1750Sensor, factor, 1);
    capabilities.push_back(luminositySensorCapability);
    return *luminositySensorCapability;
}

BH1750SensorCapability &CollectionCapabilityBuilder::addLuminositySensorCapability(BH1750 *bh1750Sensor, int factor, float readInterval)
{
    BH1750SensorCapability *luminositySensorCapability = new BH1750SensorCapability(bh1750Sensor, factor, readInterval);
    capabilities.push_back(luminositySensorCapability);
    return *luminositySensorCapability;
}

BH1750SensorCapability &CollectionCapabilityBuilder::addLuminositySensorCapability(int factor, float readInterval)
{
    BH1750SensorCapability *luminositySensorCapability = new BH1750SensorCapability(factor, readInterval);
    capabilities.push_back(luminositySensorCapability);
    return *luminositySensorCapability;
}
#endif

DistanceSensorCapability &CollectionCapabilityBuilder::addHC_SR04DistanceSensorCapability(int trigPin, int echoPin)
{
    DistanceSensorCapability *distanceSensorCapability = new DistanceSensorCapability(trigPin, echoPin, SensorModel::HC_SR04);
    capabilities.push_back(distanceSensorCapability);
    return *distanceSensorCapability;
}

DistanceSensorCapability &CollectionCapabilityBuilder::addHC_SR04DistanceSensorCapability(int trigPin, int echoPin, long minDistance, long maxDistance)
{
    DistanceSensorCapability *distanceSensorCapability = new DistanceSensorCapability(trigPin, echoPin, SensorModel::HC_SR04);
    distanceSensorCapability->setMinDistance(minDistance);
    distanceSensorCapability->setMaxDistance(maxDistance);
    capabilities.push_back(distanceSensorCapability);
    return *distanceSensorCapability;
}

#ifdef DHT_ENABLED
HumiditySensorCapability &CollectionCapabilityBuilder::addHumiditySensorCapability(DHT *dht, int intervalMinutes)
{
    HumiditySensorCapability *humiditySensor = new HumiditySensorCapability(dht, intervalMinutes);
    capabilities.push_back(humiditySensor);
    return *humiditySensor;
}

TemperatureSensorCapability &CollectionCapabilityBuilder::addTemperatureSensorCapability(DHT *dht, int intervalMinutes)
{
    TemperatureSensorCapability *temperatureSensor = new TemperatureSensorCapability(dht, intervalMinutes);
    capabilities.push_back(temperatureSensor);
    return *temperatureSensor;
}
#endif

#ifdef DS18B20_ENABLED
TemperatureSensorCapability &CollectionCapabilityBuilder::addTemperatureSensorCapability(int pin, int intervalMinutes)
{
    TemperatureSensorCapability *temperatureSensor = new TemperatureSensorCapability(pin, intervalMinutes);
    capabilities.push_back(temperatureSensor);
    return *temperatureSensor;
}
#endif

WaterLevelPercentCapability &CollectionCapabilityBuilder::addWaterLevelPercentageCapability(String capability_name, WaterLevelSensor *sensor)
{
    WaterLevelPercentCapability *waterLevelPercentageCapability = new WaterLevelPercentCapability(capability_name, sensor);
    capabilities.push_back(waterLevelPercentageCapability);
    return *waterLevelPercentageCapability;
}

WaterLevelPercentCapability &CollectionCapabilityBuilder::addWaterLevelPercentageCapability(WaterLevelSensor *sensor)
{
    WaterLevelPercentCapability *waterLevelPercentageCapability = new WaterLevelPercentCapability(sensor);
    capabilities.push_back(waterLevelPercentageCapability);
    return *waterLevelPercentageCapability;
}

WaterLevelLitersCapability &CollectionCapabilityBuilder::addWaterLevelLitersCapability(String capability_name, WaterLevelSensor *sensor)
{
    WaterLevelLitersCapability *waterLevelLitersCapability = new WaterLevelLitersCapability(capability_name, sensor);
    capabilities.push_back(waterLevelLitersCapability);
    return *waterLevelLitersCapability;
}

WaterLevelLitersCapability &CollectionCapabilityBuilder::addWaterLevelLitersCapability(WaterLevelSensor *sensor)
{
    WaterLevelLitersCapability *waterLevelLitersCapability = new WaterLevelLitersCapability("", sensor);
    capabilities.push_back(waterLevelLitersCapability);
    return *waterLevelLitersCapability;
}

HeightWaterLevelCapability &CollectionCapabilityBuilder::addWaterHeightCapability(String capability_name, WaterLevelSensor *sensor)
{
    HeightWaterLevelCapability *waterLevelLitersCapability = new HeightWaterLevelCapability(capability_name, sensor);
    capabilities.push_back(waterLevelLitersCapability);
    return *waterLevelLitersCapability;
}

HeightWaterLevelCapability &CollectionCapabilityBuilder::addWaterHeightCapability(WaterLevelSensor *sensor)
{
    HeightWaterLevelCapability *waterLevelLitersCapability = new HeightWaterLevelCapability("", sensor);
    capabilities.push_back(waterLevelLitersCapability);
    return *waterLevelLitersCapability;
}

WaterFlowHallSensorCapability &CollectionCapabilityBuilder::addWaterFlowHallSensorCapability(String capability_name, int pin)
{
    WaterFlowHallSensorCapability *waterFlowHallSensorCapability = new WaterFlowHallSensorCapability(capability_name, pin);
    capabilities.push_back(waterFlowHallSensorCapability);
    return *waterFlowHallSensorCapability;
}

WaterFlowHallSensorCapability &CollectionCapabilityBuilder::addWaterFlowHallSensorCapability(String capability_name, int pin, ValveCapability *valve)
{
    WaterFlowHallSensorCapability *waterFlowHallSensorCapability = new WaterFlowHallSensorCapability(capability_name, pin, valve);
    capabilities.push_back(waterFlowHallSensorCapability);
    return *waterFlowHallSensorCapability;
}

std::vector<Capability *> CollectionCapabilityBuilder::build()
{
    return capabilities;
}

bool CollectionCapabilityBuilder::hasBuiltCapabilities()
{
    return hasFree;
}

bool CollectionCapabilityBuilder::canBuild()
{
    return capabilities.size() > 0;
}
