#include <Arduino.h>
#include "Utils/Logger.h"

int pinoDIR = 12;
int pinoSTEP = 14;


void setup()
{
    pinMode(pinoDIR, OUTPUT);
    pinMode(pinoSTEP, OUTPUT);

    LOG_BEGIN(115200);

    digitalWrite(pinoDIR, HIGH); 

    delay(1000); 
    
    LOG_PRINTLN("Movimento completo. Parado.");
}

void loop()
{
    
        
        for (int i = 0; i < 100; i++)
        {
            digitalWrite(pinoSTEP, HIGH);
            delay(10); 
            digitalWrite(pinoSTEP, LOW);
            delay(10);
        }

        delay(3000); 
    
}
