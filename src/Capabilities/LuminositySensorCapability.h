#pragma once

#include <Arduino.h>
#include "Capability.h"

class LuminositySensorCapability : public Capability
{
public:
    LuminositySensorCapability(float luminosidadeVariationToleranceToUpdate, float readInterval = 1);
    LuminositySensorCapability(String capability_name, float luminosidadeVariationToleranceToUpdate, float readInterval = 1);

    void setup() override;
    void handle() override;

    float getLightLevel();

protected:
    virtual float readLightLevel() = 0;

private:
    unsigned long lastReadTime;
    float luminosidadeAmbiente;
    float luminosidadeVariationToleranceToUpdate;
    long readInterval;
};