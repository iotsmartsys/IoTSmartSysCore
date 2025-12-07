#include <Arduino.h>
#ifdef BH1750_ENABLED
#include "BH1750SensorCapability.h"
#include "Utils/Logger.h"

BH1750 bh1750Sensor;


BH1750SensorCapability::BH1750SensorCapability(BH1750 *lightMeter, float readInterval) : LuminositySensorCapability(0, readInterval)
{
    this->lightMeter = lightMeter;
}

BH1750SensorCapability::BH1750SensorCapability(BH1750 *lightMeter, float luminosidadeVariationToleranceToUpdate, float readInterval) : LuminositySensorCapability(luminosidadeVariationToleranceToUpdate, readInterval)
{
    this->lightMeter = lightMeter;
}

BH1750SensorCapability::BH1750SensorCapability(float luminosidadeVariationToleranceToUpdate, float readInterval) : LuminositySensorCapability(luminosidadeVariationToleranceToUpdate, readInterval)
{
    this->lightMeter = &bh1750Sensor;
}

void BH1750SensorCapability::setup()
{
    LOG_PRINTLN("BH1750SensorCapability setup");
    Wire.begin();
    lightMeter->begin();
    LOG_PRINTLN("BH1750SensorCapability setup done");
}

float BH1750SensorCapability::readLightLevel()
{
    return lightMeter->readLightLevel();
}
#endif 
