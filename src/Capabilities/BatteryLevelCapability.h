#pragma once

#include <Arduino.h>
#include "Capability.h"


class BatteryLevelCapability : public Capability
{
public:
    BatteryLevelCapability(String capability_name, int pin);

    void handle() override;
    void setup() override;

private:
    int pin;
    float batteryLevel;
    long lastTime;
    const long interval;
    int PIN_BATTERY_ADC;

    const float R1;
    const float R2;
    const float VREF;
    const int ADC_RESOLUTION;

    int readStableADC(int pin, int samples = 10);
    void handleBatteryLevel();
};