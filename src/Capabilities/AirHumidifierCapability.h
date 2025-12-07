#pragma once

#include <Arduino.h>
#include "Capability.h"

#ifdef IRREMOTE_ENABLED
#include <IRsend.h>

class AirHumidifierCapability : public Capability
{
public:
    AirHumidifierCapability(int irPin);

    AirHumidifierCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

private:
    int irPin;
    IRsend irsend;

    void executeCommand(String state);
    void sendOnOff();
    void sendContinuous();
    void sendBigSmall();
    void sendStart();
};
#endif