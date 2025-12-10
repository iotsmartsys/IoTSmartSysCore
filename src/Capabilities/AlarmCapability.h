#pragma once

#include "Capability.h"

class AlarmCapability : public Capability
{
public:
    AlarmCapability(int alarmPin, DigitalLogic stateLogic = DigitalLogic::NORMAL);

    AlarmCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    bool isActivated();

    
    void ring();
    void setRingDuration(int duration);

    void powerOn();
    void powerOff();

    void executeCommand(String state);

private:
    int alarmPin;
    int stateOn;
    int stateOff;
    unsigned long lastRing;
    bool poweredOn;
    bool lastState;
    int ringDuration = 250;

    void toggle();
};
