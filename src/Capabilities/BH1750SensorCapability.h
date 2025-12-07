#pragma once

#include <Arduino.h>
#ifdef BH1750_ENABLED
#include <BH1750.h>
#include "Capability.h"
#include "LuminositySensorCapability.h"



class BH1750SensorCapability : public LuminositySensorCapability
{
public:
    BH1750SensorCapability(BH1750 *lightMeter, float readInterval = 1);

    BH1750SensorCapability(BH1750 *lightMeter, float luminosidadeVariationToleranceToUpdate, float readInterval = 1);
    BH1750SensorCapability(float luminosidadeVariationToleranceToUpdate, float readInterval = 1);

    float readLightLevel() override;
    void setup() override;

private: 
BH1750 *lightMeter;
    float luminosidadeAmbiente = 0;
    float luminosidadeVariationToleranceToUpdate = 0;
};
#endif 