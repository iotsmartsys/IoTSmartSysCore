#include <Arduino.h>
#include "Utils/Logger.h"

const int gpioStart = 43;
const int gpioEnd = 45; 

void setup()
{
    LOG_BEGIN(115200);
    delay(1000);
    LOG_PRINTLN("🔎 Iniciando varredura de GPIOs para encontrar LEDs...");

    for (int pin = gpioStart; pin <= gpioEnd; pin++)
    {
        
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        LOG_PRINTF("👉 Testando GPIO %d\n", pin);
        delay(300);
        digitalWrite(pin, LOW);
        delay(100);
    }

    LOG_PRINTLN("✅ Varredura finalizada.");
}

void loop()
{
    
}
