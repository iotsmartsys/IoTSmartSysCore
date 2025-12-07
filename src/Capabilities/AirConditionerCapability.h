#pragma once
#ifdef IRREMOTE_ENABLED
#include <Arduino.h>
#include <IRrecv.h>
#include "Capability.h"


class IRrecv;
struct decode_results;

class AirConditionerCapability : public Capability
{
public:
    AirConditionerCapability(int irPin);

    AirConditionerCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    bool irSensorIsTriggered();

private:
    IRrecv *irrecv;
    decode_results results;
    int irPin;
    uint64_t lastState;
    long lastSendEvent;

    void handleIRSensor();
};

#endif