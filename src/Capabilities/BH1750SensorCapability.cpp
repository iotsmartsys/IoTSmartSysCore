#include <Arduino.h>
#ifdef BH1750_ENABLED
#include "BH1750SensorCapability.h"
#include "Utils/Logger.h"

namespace {
    BH1750 defaultBh1750;
}


BH1750SensorCapability::BH1750SensorCapability(BH1750 *lightMeter, float readInterval)
    : BH1750SensorCapability(lightMeter, 0.0f, readInterval)
{
}

BH1750SensorCapability::BH1750SensorCapability(BH1750 *lightMeter,
                                               float luminosidadeVariationToleranceToUpdate,
                                               float readInterval)
    : LuminositySensorCapability(luminosidadeVariationToleranceToUpdate, readInterval),
      lightMeter(lightMeter ? lightMeter : &defaultBh1750)
{
}

BH1750SensorCapability::BH1750SensorCapability(float luminosidadeVariationToleranceToUpdate,
                                               float readInterval)
    : BH1750SensorCapability(&defaultBh1750,
                             luminosidadeVariationToleranceToUpdate,
                             readInterval)
{
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
