#include "LuminositySensorCapability.h"

LuminositySensorCapability::LuminositySensorCapability(float luminosidadeVariationToleranceToUpdate, float readInterval)
    : Capability(LIGHT_SENSOR_TYPE, "-1"), lastReadTime(0), luminosidadeAmbiente(0), luminosidadeVariationToleranceToUpdate(luminosidadeVariationToleranceToUpdate), readInterval(readInterval * 1000)
{
}

LuminositySensorCapability::LuminositySensorCapability(String capability_name, float luminosidadeVariationToleranceToUpdate, float readInterval)
    : Capability(capability_name, LIGHT_SENSOR_TYPE, "-1"), lastReadTime(0), luminosidadeAmbiente(0), luminosidadeVariationToleranceToUpdate(luminosidadeVariationToleranceToUpdate), readInterval(readInterval * 1000)
{
}

void LuminositySensorCapability::setup()
{
    LOG_PRINTLN("Setup LuminositySensorCapability");
}

void LuminositySensorCapability::handle()
{
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readInterval)
    {
        return; 
    }

    lastReadTime = currentTime;

    float lux = this->readLightLevel();
    LOG_PRINTF("Luminosidade: %f lux\r\n", lux);
    if (luminosidadeAmbiente != lux)
    {
        if (abs(luminosidadeAmbiente - lux) > luminosidadeVariationToleranceToUpdate)
        {
            luminosidadeAmbiente = lux;
            updateState(String(luminosidadeAmbiente));
        }
    }
}

float LuminositySensorCapability::getLightLevel()
{
    return luminosidadeAmbiente;
}
