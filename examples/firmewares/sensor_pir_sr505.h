#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include "Builders/CapabilityBuilder.h"
#include <math.h>

#define LED_BUILTIN 2
#define PIR_PIN 19

CollectionCapabilityBuilder *builder = new CollectionCapabilityBuilder();

PirSensorCapability pirSensorCapability(PIR_PIN, 0); 

std::vector<Capability *> capabilities;
IoTCore *iotCore;


void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");

    pinMode(PIR_PIN, INPUT);      
    pinMode(LED_BUILTIN, OUTPUT); 
    
}

void loop()
{
    if (digitalRead(PIR_PIN) == HIGH)
    {
        LOG_PRINTLN("PIR - Motion detected!");
        digitalWrite(LED_BUILTIN, HIGH); 
    }
    else
    {
        LOG_PRINTLN("PIR - No motion detected.");
        digitalWrite(LED_BUILTIN, LOW); 
    }
    
    delay(1000); 
}
