#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include "Builders/CapabilityBuilder.h"
#include <math.h>

#define LED_BUILTIN 2
#define PIR_PIN 13
#define DHT11_PIN 23
#define IR_RECEIVE_PIN 4

#define WATER_LEVEL_SENSOR_PIN_TRIGGER 19
#define WATER_LEVEL_SENSOR_PIN_ECHO 21

#ifdef DHT_ENABLED
DHT dht(DHT11_PIN, DHT11);
#endif 

CollectionCapabilityBuilder *builder = new CollectionCapabilityBuilder();

DistanceSensorCapability distance(WATER_LEVEL_SENSOR_PIN_TRIGGER, WATER_LEVEL_SENSOR_PIN_ECHO, SensorModel::HC_SR04); 

std::vector<Capability *> capabilities;
IoTCore *iotCore;


void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");

    
    
    
    
    
    
    

    

    

    distance.setup();

    
    
}

void loop()
{
    
    distance.handle();
    delay(1000); 
}
