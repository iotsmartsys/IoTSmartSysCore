#pragma once

#include <Arduino.h>

class WaterLevelSensor
{
public:
    
    virtual void setup() = 0;
    virtual void handleSensor() = 0;
    virtual float getLevelPercent() = 0;
    virtual float getLevelLiters() = 0;
    virtual float getHeightWaterInCm() = 0;

    
    void handle()
    {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime >= readIntervalMs)
        {
            lastReadTime = currentTime;
            handleSensor();
        }
    }

    void setReadIntervalMs(unsigned long intervalMs)
    {
        this->readIntervalMs = intervalMs;
    }

    void setReadIntervalMinute(unsigned long intervalMinute)
    {
        this->readIntervalMs = intervalMinute * 60000;
    }

protected:
    unsigned long lastReadTime = 0;
    unsigned long readIntervalMs = 0;
};
