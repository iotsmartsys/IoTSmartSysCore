#include <Arduino.h>
#include "Utils/Logger.h"

#define VBAT_PIN 35 

unsigned long startMillis;
unsigned long eventCount = 0;

float readVBat()
{
    const int samples = 10;
    long sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += analogRead(VBAT_PIN);
        delay(2);
    }
    float raw = sum / (float)samples;
    float voltage = (raw / 4095.0) * 3.3; 
    voltage *= 2.0;                       
    return voltage;
}

void setup()
{
    LOG_BEGIN(115200);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); 
    startMillis = millis();
}

void loop()
{
    float vbat = readVBat();
    unsigned long uptimeSecs = (millis() - startMillis) / 1000;
    eventCount++;

    LOG_PRINTF("VBAT: %.3f V | Uptime: %lu s | Evento: %lu\n",
                  vbat, uptimeSecs, eventCount);

    delay(60000); 
}
