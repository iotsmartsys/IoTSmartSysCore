#include "AirConditionerCapability.h"

#ifdef IRREMOTE_ENABLED
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include "Infra/Sensors/IR/AirConditionerCod.h"

AirConditionerCapability::AirConditionerCapability(int irPin)
    : Capability("", AIR_CONDITIONER_TYPE, "OFF"), irrecv(nullptr), irPin(irPin), lastState(0), lastSendEvent(0)
{
    irrecv = new IRrecv(irPin);
}

AirConditionerCapability::AirConditionerCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value), irrecv(nullptr), irPin(-1), lastState(0), lastSendEvent(0)
{
    irrecv = new IRrecv(irPin);
}

void AirConditionerCapability::setup()
{
    if (irrecv)
    {
        irrecv->enableIRIn(); 
        LOG_PRINTLN("Aguardando sinal IR...");
    }
}

void AirConditionerCapability::handle()
{
    handleIRSensor();
}

bool AirConditionerCapability::irSensorIsTriggered()
{
    return digitalRead(this->irPin) == LOW;
}

void AirConditionerCapability::handleIRSensor()
{
    if (!irrecv)
        return;

    if (irrecv->decode(&results))
    {
        
        LOG_PRINT("Código recebido: 0x");
        uint64_t currentValue = results.value;

        LOG_PRINTLN(results.value);

        LOG_PRINT("resultToSourceCode: 0X");
        lastState = currentValue;
        String hexResult = getACMode(currentValue);

        if (hexResult != "unknown")
            updateState(hexResult);

        LOG_PRINTLN(resultToHumanReadableBasic(&results));

        irrecv->resume();
    }
}

#endif
