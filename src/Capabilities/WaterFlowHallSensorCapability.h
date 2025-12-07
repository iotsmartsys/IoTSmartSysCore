#pragma once
#include <Arduino.h>
#include "Capability.h"
#include "ValveCapability.h"

class WaterFlowHallSensorCapability : public Capability
{
public:
    WaterFlowHallSensorCapability(String capability_name, int pin);

    WaterFlowHallSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);
    WaterFlowHallSensorCapability(String capability_name, int pin, ValveCapability *valve);

    static WaterFlowHallSensorCapability *instance;

    void setup() override;
    void handle() override;
    void incrementPulseCount();
    float getTotalLiters() const { return totalLiters; }

private:
    int pin;
    int lastDoorState = 0;
    volatile unsigned int pulseCount = 0;
    unsigned long lastMillis = 0;
    float flowRate = 0.0;
    float totalLiters = 0.0;
    float lastTotalLiters = 0.0;

    ValveCapability *valveRef;
};

void IRAM_ATTR waterFlowPulseISR(); 