#include "IoTSmartSysCore.h"

#include "Platform/Builders/CapabilityBuilder.h"

void IoTCore::capabilitiesHandle()
{
    for (const auto &cap : capabilities)
    {
        try
        {
            cap->handle();

            if (cap->hasChanged())
            {
                LOG_PRINTLN("[IoTCore] Mudança detectada na capability capability_name: " + cap->capability_name);
                LOG_PRINTLN("[IoTCore] Obtendo data e hora atual...");
                LOG_PRINTLN("[IoTCore] Enviando estado da capability...");
                sendState(cap->readState());
                LOG_PRINTLN("[IoTCore] Estado da capability enviado.");
            }
        }
        catch (const std::exception &e)
        {
            LOG_PRINTLN("[IoTCore] Erro ao processar capability: " + cap->capability_name);
            std::cerr << e.what() << '\n';
        }
    }
}

Capability &IoTCore::addCapability(Capability *capability)
{
    return capabilityBuilder->addCapability(capability);
}

#ifdef GLP_METER_ENABLED
GlpMeterCapability &IoTCore::addGlpMeterCapability(String capability_name, int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg)
{
    return capabilityBuilder->addGlpMeterCapability(capability_name, dout_pin, sck_pin, tare_weight_kg, weight_capacity_kg);
}

GlpMeterCapability &IoTCore::addGlpMeterCapability(int dout_pin, int sck_pin, float tare_weight_kg, float weight_capacity_kg)
{
    return capabilityBuilder->addGlpMeterCapability(dout_pin, sck_pin, tare_weight_kg, weight_capacity_kg);
}
#endif

#ifdef RGB_ENABLED
RGBLightCapability &IoTCore::addRGBLightCapability(int pin, int red, int green, int blue)
{
    return capabilityBuilder->addRGBLightCapability(pin, red, green, blue);
}
#endif

#ifdef OPERATIONAL_COLOR_SENSOR_ENABLED
OperationalColorSensorCapability &IoTCore::addOperationalColorSensorCapability(unsigned int readIntervalMs)
{
    return capabilityBuilder->addOperationalColorSensorCapability(readIntervalMs);
}
#endif

#ifdef IRREMOTE_ENABLED
AirHumidifierCapability &IoTCore::addAirHumidifierCapability(int irPin)
{
    return capabilityBuilder->addAirHumidifierCapability(irPin);
}
AirConditionerCapability &IoTCore::addAirConditionerCapability(int irPin)
{
    return capabilityBuilder->addAirConditionerCapability(irPin);
}
#endif

ClapSensorCapability &IoTCore::addClapSensorCapability(String capability_name, int clapPin, int toleranceTime)
{
    return capabilityBuilder->addClapSensorCapability(capability_name, clapPin, toleranceTime);
}

SwitchCapability &IoTCore::addSwitchCapability(String capability_name, int switchPin, DigitalLogic switchLogic)
{
    return capabilityBuilder->addSwitchCapability(capability_name, switchPin, switchLogic);
}

SwitchCapability &IoTCore::addSwitchCapability(int switchPin, DigitalLogic switchLogic)
{
    return capabilityBuilder->addSwitchCapability(switchPin, switchLogic);
}

SwitchPlugCapability &IoTCore::addSwitchPlugCapability(String capability_name, int switchPin, DigitalLogic switchLogic)
{
    return capabilityBuilder->addSwitchPlugCapability(capability_name, switchPin, switchLogic);
}

SwitchPlugCapability &IoTCore::addSwitchPlugCapability(int switchPin, DigitalLogic switchLogic)
{
    return capabilityBuilder->addSwitchPlugCapability(switchPin, switchLogic);
}

IRProximitySensorCapability &IoTCore::addIRProximitySensorCapability(String capability_name, int irPin)
{
    return capabilityBuilder->addIRProximitySensorCapability(capability_name, irPin);
}

PirSensorCapability &IoTCore::addPirSensorCapability(int pirPin, int toleranceTime)
{
    return capabilityBuilder->addPirSensorCapability(pirPin, toleranceTime);
}

PirSensorCapability &IoTCore::addPirSensorCapability(String capability_name, int pirPin, int toleranceTime)
{
    PirSensorCapability *pirSensorCapability = &capabilityBuilder->addPirSensorCapability(pirPin, toleranceTime);
    pirSensorCapability->rename(capability_name);
    return *pirSensorCapability;
}

DoorSensorCapability &IoTCore::addDoorSensorCapability(int doorPin)
{
    return capabilityBuilder->addDoorSensorCapability(doorPin);
}

LEDCapability &IoTCore::addLEDCapability(String capability_name, int ledPin, DigitalLogic ledLogic)
{
    return capabilityBuilder->addLEDCapability(capability_name, ledPin, ledLogic);
}

ValveCapability &IoTCore::addValveCapability(String capability_name, int valvePin, DigitalLogic valveLogic)
{
    return capabilityBuilder->addValveCapability(capability_name, valvePin, valveLogic);
}

#ifdef BH1750_ENABLED
BH1750SensorCapability &IoTCore::addLuminositySensorCapability(int factor, float readInterval)
{
    return capabilityBuilder->addLuminositySensorCapability(factor, readInterval);
}
#endif

DistanceSensorCapability &IoTCore::addHC_SR04DistanceSensorCapability(int trigPin, int echoPin)
{
    return capabilityBuilder->addHC_SR04DistanceSensorCapability(trigPin, echoPin);
}

DistanceSensorCapability &IoTCore::addHC_SR04DistanceSensorCapability(int trigPin, int echoPin, long minDistance, long maxDistance)
{
    return capabilityBuilder->addHC_SR04DistanceSensorCapability(trigPin, echoPin, minDistance, maxDistance);
}

#ifdef DHT_ENABLED
HumiditySensorCapability &IoTCore::addHumiditySensorCapability(DHT *dht, int intervalMinutes)
{
    return capabilityBuilder->addHumiditySensorCapability(dht, intervalMinutes);
}
TemperatureSensorCapability &IoTCore::addTemperatureSensorCapability(DHT *dht, int intervalMinutes)
{
    return capabilityBuilder->addTemperatureSensorCapability(dht, intervalMinutes);
}
#endif

#ifdef DS18B20_ENABLED
TemperatureSensorCapability &IoTCore::addTemperatureSensorCapability(int pin, int intervalMinutes)
{
    return capabilityBuilder->addTemperatureSensorCapability(pin, intervalMinutes);
}

TemperatureSensorCapability &IoTCore::addTemperatureSensorCapability(String capability_name, int pin, int intervalMinutes)
{
    TemperatureSensorCapability *cap = &capabilityBuilder->addTemperatureSensorCapability(pin, intervalMinutes);
    cap->rename(capability_name);
    return *cap;
}
#endif

WaterLevelPercentCapability &IoTCore::addWaterLevelPercentageCapability(String capability_name, WaterLevelSensor *sensor)
{
    return capabilityBuilder->addWaterLevelPercentageCapability(capability_name, sensor);
}

WaterLevelPercentCapability &IoTCore::addWaterLevelPercentageCapability(WaterLevelSensor *sensor)
{
    return capabilityBuilder->addWaterLevelPercentageCapability(sensor);
}

WaterLevelLitersCapability &IoTCore::addWaterLevelLitersCapability(String capability_name, WaterLevelSensor *sensor)
{
    return capabilityBuilder->addWaterLevelLitersCapability(capability_name, sensor);
}

WaterLevelLitersCapability &IoTCore::addWaterLevelLitersCapability(WaterLevelSensor *sensor)
{
    return capabilityBuilder->addWaterLevelLitersCapability(sensor);
}

HeightWaterLevelCapability &IoTCore::addWaterHeightCapability(String capability_name, WaterLevelSensor *sensor)
{
    return capabilityBuilder->addWaterHeightCapability(capability_name, sensor);
}

HeightWaterLevelCapability &IoTCore::addWaterHeightCapability(WaterLevelSensor *sensor)
{
    return capabilityBuilder->addWaterHeightCapability(sensor);
}

WaterFlowHallSensorCapability &IoTCore::addWaterFlowHallSensorCapability(String capability_name, int pin)
{
    return capabilityBuilder->addWaterFlowHallSensorCapability(capability_name, pin);
}

WaterFlowHallSensorCapability &IoTCore::addWaterFlowHallSensorCapability(String capability_name, int pin, ValveCapability *valve)
{
    return capabilityBuilder->addWaterFlowHallSensorCapability(capability_name, pin, valve);
}
