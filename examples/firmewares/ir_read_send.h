#include <Arduino.h>
#include "Utils/Logger.h"
#include "Core/IoTCore.h"
#include "Builders/CapabilityBuilder.h"
#include <math.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

#define LED_BUILTIN 2
#define IR_RECEIVE_PIN 4
#define IR_LED_PIN 18

IRrecv irrecv(IR_RECEIVE_PIN);
IRsend irsend(IR_LED_PIN);
decode_results results;

void sendOnOff();
void sendContinuous();
void sendBigSmall();

void sendStart();
void handleIRSensor();
void setup()
{
    LOG_BEGIN(115200);
    LOG_PRINTLN("Setup");
    irrecv.enableIRIn();
    irsend.begin();

    
    irsend.sendKelon(0xC02000006E56, 48); 
    sendStart();

    delay(5000); 

    sendOnOff();
}

void loop()
{
    
    handleIRSensor();
    delay(500);
}

void handleIRSensor()
{
    if (irrecv.decode(&results))
    {
        
        LOG_PRINT("Código recebido: 0x");
        uint64_t currentValue = results.value;

        LOG_PRINTLN(results.value, HEX);

        LOG_PRINT("resultToSourceCode: 0X");

        LOG_PRINTLN(resultToHumanReadableBasic(&results));

        irrecv.resume();
    }
}
void sendOnOff()
{
    
    irsend.sendNEC(0xFFA25D, 32); 
    delay(100);
}
void sendContinuous()
{
    
    irsend.sendNEC(0xFFE21D, 32); 
    delay(100);
}
void sendBigSmall()
{
    
    irsend.sendNEC(0xFFA857, 32); 
    delay(100);
}

void sendStart()
{
    
    sendOnOff();
    sendContinuous();
    sendBigSmall();
    LOG_PRINTLN("Códigos IR enviados.");
}
