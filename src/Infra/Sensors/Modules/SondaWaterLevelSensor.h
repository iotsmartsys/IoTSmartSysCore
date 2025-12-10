#pragma once

#include <Arduino.h>
#include "Utils/Logger.h"
#include "Sensors/Modules/WaterLevelSensor.h"
#include <math.h>

class SondaWaterLevelSensor : public WaterLevelSensor
{
public:
    
    SondaWaterLevelSensor()
    {
    }
    
    SondaWaterLevelSensor(const int pins[], const int percents_equivalent[], unsigned int levels_quantity, unsigned long intervalMs = 0)
    {
        for (int i = 0; i < NUM_PINS; ++i)
        {
            this->sensor_pins[i] = pins[i];
        }
        for (int i = 0; i < NUM_PINS; ++i)
        {
            this->percents_equivalent[i] = percents_equivalent[i];
        }
        this->readIntervalMs = intervalMs;
        this->levels_quantity = levels_quantity;
    }

    void setup() override
    {
        for (int i = 0; i < levels_quantity; i++)
        {
            if (sensor_pins[i] == 0)
                continue;
            pinMode(sensor_pins[i], INPUT_PULLUP);
        }
    }

    float getLevelPercent()
    {
        return levelPercent;
    }

    float getLevelLiters()
    {
        return levelLiters;
    }

    float getHeightWaterInCm()
    {
        return actualHeightCM;
    }
private:
    static constexpr int NUM_PINS = 10;
    int sensor_pins[NUM_PINS] = {0};
    int percents_equivalent[NUM_PINS] = {0};
    
    unsigned int levels_quantity = 2;
    
    float actualHeightCM = 0;
    float levelPercent = 0;
    float levelLiters = 0;

    void handleSensor() override
    {
        float max_level = 0;
        for (int i = 0; i < levels_quantity; i++)
        {
            if (sensor_pins[i] == 0)
                continue;
            if (digitalRead(sensor_pins[i]) == LOW)
            {
                LOG_PRINTLN("Sensor pin " + String(sensor_pins[i]) + " detectou água.");
                float actualLevelPercent = percents_equivalent[i];
                if (actualLevelPercent > max_level)
                {
                    max_level = actualLevelPercent;
                }
                LOG_PRINTLN("Nível de água: " + String(actualLevelPercent) + "%");

                levelLiters = (actualLevelPercent / 1000.0) * 100.0; 
                
            }
        }
        levelPercent = max_level;
    }
};
