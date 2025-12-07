#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include "Builders/CapabilityBuilder.h"
#include <math.h>

BH1750 lightMeter;

#define LED_PIN 8
#define LED_BUILTIN 2 

CollectionCapabilityBuilder *builder = new CollectionCapabilityBuilder();

std::vector<Capability *> capabilities;
IoTCore *iotCore;

LEDCapability ledCapability("led", LED_BUILTIN, DigitalLogic::NORMAL);

BH1750SensorCapability *luminositySensorCapability;

void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");


    
    
    luminositySensorCapability = &builder->addLuminositySensorCapability(&lightMeter, 0, 10);

    
    luminositySensorCapability->setup();

    ledCapability.setup();
    ledCapability.turnOn();
}

void loop()
{
    luminositySensorCapability->handle();


    
}
