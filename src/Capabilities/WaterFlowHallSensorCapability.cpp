#include <Arduino.h>
#include "WaterFlowHallSensorCapability.h"
#include "ValveCapability.h"

volatile unsigned long lastPulseMicros = 0;
volatile bool previousValveState = false;

WaterFlowHallSensorCapability::WaterFlowHallSensorCapability(String capability_name, int pin)
    : Capability(capability_name, WATER_FLOW_SENSOR_TYPE, "0")
{
    this->pin = pin;
}

WaterFlowHallSensorCapability::WaterFlowHallSensorCapability(String capability_name, String description, String owner, String type, String mode, String value)
    : Capability(capability_name, description, owner, type, mode, value)
{
    pin = -1;
}

WaterFlowHallSensorCapability::WaterFlowHallSensorCapability(String capability_name, int pin, ValveCapability *valve)
    : Capability(capability_name, WATER_FLOW_SENSOR_TYPE, "0"), valveRef(valve)
{
    this->pin = pin;
}

void WaterFlowHallSensorCapability::setup()
{
    if (pin == -1)
        return;

    pinMode(pin, INPUT_PULLUP);
    instance = this;
    attachInterrupt(digitalPinToInterrupt(pin), waterFlowPulseISR, FALLING);
    lastMillis = millis();
}

void WaterFlowHallSensorCapability::handle()
{
    if (millis() - lastMillis >= 1000)
    {
        detachInterrupt(digitalPinToInterrupt(pin));
        float previousTotalLiters = totalLiters;
        float liters = pulseCount / 16274.3;
        bool currentValveState = valveRef == nullptr || valveRef->isOpen();
        if (valveRef != nullptr)
        {
            if (currentValveState && !previousValveState)
            {
                totalLiters = 0;
            }
            previousValveState = currentValveState;
        }
        if (currentValveState && liters > 0.01)
        {
            totalLiters += liters;
        }
        pulseCount = 0;
        lastMillis = millis();
        attachInterrupt(digitalPinToInterrupt(pin), waterFlowPulseISR, FALLING);

        if (totalLiters != lastTotalLiters)
        {
            lastTotalLiters = totalLiters;
            updateState(String(totalLiters, 3));
        }
    }
}

void WaterFlowHallSensorCapability::incrementPulseCount()
{
    pulseCount++;
}

WaterFlowHallSensorCapability *WaterFlowHallSensorCapability::instance = nullptr;

void IRAM_ATTR waterFlowPulseISR()
{
    if (WaterFlowHallSensorCapability::instance != nullptr)
    {
        WaterFlowHallSensorCapability::instance->incrementPulseCount();
    }
}
