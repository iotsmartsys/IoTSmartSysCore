#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include "Builders/CapabilityBuilder.h"
#include <math.h>

#define PIR_PIN 13
#define DHT11_PIN 23
#define IR_RECEIVE_PIN 4





#define ULN2003_OUT1 13
#define ULN2003_OUT2 14
#define ULN2003_OUT3 26
#define ULN2003_OUT4 27
#define ULN2003_OUT5 25
#define ULN2003_OUT6 33
#define ULN2003_OUT7 32


#ifdef DHT_ENABLED
DHT dht(DHT11_PIN, DHT11);
#endif 

CollectionCapabilityBuilder *builder = new CollectionCapabilityBuilder();

DistanceSensorCapability distance(WATER_LEVEL_SENSOR_PIN_TRIGGER, WATER_LEVEL_SENSOR_PIN_ECHO, SensorModel::HC_SR04); 

std::vector<Capability *> capabilities;
IoTCore *iotCore;
LEDCapability ledCapability("led", LED_BUILTIN, DigitalLogic::NORMAL);

void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ULN2003_OUT1, INPUT);
    pinMode(ULN2003_OUT2, INPUT);
    pinMode(ULN2003_OUT3, INPUT);
    pinMode(ULN2003_OUT4, INPUT);
    pinMode(ULN2003_OUT5, INPUT);
    pinMode(ULN2003_OUT6, INPUT);
    pinMode(ULN2003_OUT7, INPUT);   

    

    
}
unsigned long count = 0;
void loop()
{
    LOG_PRINTLN("Loop count: " + String(count++));
    LOG_PRINTLN("Reading ULN2003 OUT1: " + String(digitalRead(ULN2003_OUT1)));

    if (digitalRead(ULN2003_OUT1) == LOW)
    {
        LOG_PRINTLN("ULN2003 OUT1 is LOW");
        digitalWrite(LED_BUILTIN, HIGH); 
    }
    else
    {
        LOG_PRINTLN("ULN2003 OUT1 is HIGH");
        digitalWrite(LED_BUILTIN, LOW); 
    }

}
