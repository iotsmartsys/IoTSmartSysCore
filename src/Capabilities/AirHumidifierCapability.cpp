#include "AirHumidifierCapability.h"
#include "Utils/Logger.h"
#ifdef IRREMOTE_ENABLED
#include <IRremoteESP8266.h>
#include <IRutils.h>

AirHumidifierCapability::AirHumidifierCapability(int irPin)
    : Capability("", AIR_HUMIDIFIER_TYPE, "Off"), irPin(irPin), irsend(irPin)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                    executeCommand(*state);
                });
}

AirHumidifierCapability::AirHumidifierCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), irPin(-1), irsend(0)
{
}

void AirHumidifierCapability::setup()
{
    irsend.begin();
    LOG_PRINTLN("Aguardando sinal IR...");
}

void AirHumidifierCapability::handle()
{
}

void AirHumidifierCapability::executeCommand(String state)
{
    if (state == POWER_ON_COMMAND)
    {
        sendStart();
        updateState("On");
    }
    else if (state == POWER_OFF_COMMAND)
    {
        sendStart();
        updateState("Off");
    }
    else if (state == TOGGLE_COMMAND)
    {
        if (this->value == "On")
            updateState("Off");
        else
            updateState("On");
    }
}

void AirHumidifierCapability::sendOnOff()
{
    
    irsend.sendNEC(0xFFA25D, 32); 
    delay(100);
}

void AirHumidifierCapability::sendContinuous()
{
    
    irsend.sendNEC(0xFFE21D, 32); 
    delay(100);
}

void AirHumidifierCapability::sendBigSmall()
{
    
    irsend.sendNEC(0xFFA857, 32); 
    delay(100);
}

void AirHumidifierCapability::sendStart()
{
    
    sendOnOff();
    sendContinuous();
    sendBigSmall();
    LOG_PRINTLN("Códigos IR enviados.");
}

#endif
