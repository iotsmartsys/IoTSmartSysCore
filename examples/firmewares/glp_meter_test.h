#include <Arduino.h>
#include "../../src/Core/IoTCore.h"
#ifdef ST7789_170x320_ENABLED
#include <Arduino.h>
#include <HX711.h>

#include "../../src/display/Display_ST7789_170_320.h"

#define DOUT 19
#define SCK 5
#define TARE_WEIGHT_KG 13.4f
#define WEIGHT_CAPACITY_KG 13.50f

IoTCore *iotCore = new IoTCore();
GlpMeterCapability *glpMeterCapability;

void setup()
{
    Serial.begin(115200);
    setup_ST7789_170x320();
    delay(2000);

    glpMeterCapability = &iotCore->addGlpMeterCapability(DOUT, SCK, TARE_WEIGHT_KG, WEIGHT_CAPACITY_KG);
    
    iotCore->setup();
    displayInfo(getDeviceId());
    displayMessage("Version: " + String(IOT_PRIVATE_HOME_VERSION));

}
// 16Kg de 30Kg é 
float lastKg = 0.0;
float lastPercent = 0.0;
void loop()
{

    iotCore->handle();
    float kg = glpMeterCapability->getKg();
    float percent = glpMeterCapability->getPercent();

    if (kg != lastKg)
    {
        lastKg = kg;
    }
    if (percent != lastPercent)
    {
        displayWarning("NIVEL DE GAS");
        displayMessage("Peso: " + String(kg, 3)+ "Kg");
        displayMessage("Percentual: " + String(percent, 3) + "%");
        lastPercent = percent;
    }
}

#endif
