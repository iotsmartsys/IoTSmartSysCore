#include "IRComandCapability.h"
#include "Utils/Logger.h"
#include <Arduino.h>

IRCommandCapability::IRCommandCapability(String capability_name, int irPin)
    : Capability(capability_name, IR_TYPE, ""), irPin(irPin), currentState(0), lastState(0)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                });
}

IRCommandCapability::IRCommandCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), irPin(-1), currentState(0), lastState(0)
{
}

void IRCommandCapability::setup()
{
    pinMode(irPin, OUTPUT);
}

void IRCommandCapability::handle()
{
    
}

void IRCommandCapability::setACMode(String mode, uint8_t temp)
{
    uint64_t code = 0;

    if (mode == "off")
        code = 0xC02000006E56;
    else if (mode == "cool")
    {
        switch (temp)
        {
        case 18:
            code = 0x2000006E56;
            break;
        case 19:
            code = 0x2000006F56;
            break;
        case 20:
            code = 0x2000007056;
            break;
        case 21:
            code = 0x2000007156;
            break;
        case 22:
            code = 0x2000007256;
            break;
        case 23:
            code = 0x2000007356;
            break;
        case 24:
            code = 0x2000007456;
            break;
        }
    }
    else if (mode == "dry")
        code = 0x3300007356;
    else if (mode == "fan")
        code = 0x5200007456;
    else if (mode == "heat")
        code = 0x1000007556;
    else if (mode == "auto")
        code = 0x4000007356;

    if (code != 0)
    {
        
        LOG_PRINTF("Comando IR enviado: %llX\n", code);
    }
    else
    {
        LOG_PRINTLN("Código não reconhecido.");
    }
}
