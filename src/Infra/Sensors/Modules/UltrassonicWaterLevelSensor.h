#pragma once

#include <Arduino.h>
#include "Sensors/Modules/SensorUltrassonic_HC_SR04.h"
#include "Sensors/Modules/SensorModel.h"
#include "Sensors/Modules/WaterLevelSensor.h"
#include <math.h>

#define WATER_LEVEL_SENSOR_PIN_TRIGGER 4
#define WATER_LEVEL_SENSOR_PIN_ECHO 5

class UltrassonicWaterLevelSensor : public WaterLevelSensor
{
public:
    
    UltrassonicWaterLevelSensor(WaterLevelRecipentType recipentType)
    {
        this->trigPin = WATER_LEVEL_SENSOR_PIN_TRIGGER;
        this->echoPin = WATER_LEVEL_SENSOR_PIN_ECHO;
        this->recipentType = recipentType;
        switch (recipentType)
        {
        case WaterLevelRecipentType::Circle1000L:
            this->diameterTopCM = RECIPENT_1000L_DIAMETER_TOP;
            this->diameterBaseCM = RECIPENT_1000L_DIAMETER_BASE;
            this->heightCM = RECIPENT_1000L_HEIGHT;
            this->distanceToSensorCM = RECIPENT_1000L_DISTANCE_TO_SENSOR;
            break;
        case WaterLevelRecipentType::Circle2000L:
            this->diameterTopCM = RECIPENT_2000L_DIAMETER_TOP;
            this->diameterBaseCM = RECIPENT_2000L_DIAMETER_BASE;
            this->heightCM = RECIPENT_2000L_HEIGHT;
            this->distanceToSensorCM = RECIPENT_2000L_DISTANCE_TO_SENSOR;
            break;
        default:
            break;
        }

        sr04SensorUltrassonicCapability = new SensorUltrassonic_HC_SR04(trigPin, echoPin);
    }
    
    UltrassonicWaterLevelSensor(int trigPin, int echoPin, WaterLevelRecipentType recipentType, unsigned long intervalMs = 0)
    {
        this->trigPin = trigPin;
        this->echoPin = echoPin;
        this->recipentType = recipentType;
        this->readIntervalMs = intervalMs;
        switch (recipentType)
        {
        case WaterLevelRecipentType::Circle1000L:
            this->diameterTopCM = RECIPENT_1000L_DIAMETER_TOP;
            this->diameterBaseCM = RECIPENT_1000L_DIAMETER_BASE;
            this->heightCM = RECIPENT_1000L_HEIGHT;
            this->distanceToSensorCM = RECIPENT_1000L_DISTANCE_TO_SENSOR;
            break;
        case WaterLevelRecipentType::Circle2000L:
            this->diameterTopCM = RECIPENT_2000L_DIAMETER_TOP;
            this->diameterBaseCM = RECIPENT_2000L_DIAMETER_BASE;
            this->heightCM = RECIPENT_2000L_HEIGHT;
            this->distanceToSensorCM = RECIPENT_2000L_DISTANCE_TO_SENSOR;
            break;
        default:
            break;
        }

        sr04SensorUltrassonicCapability = new SensorUltrassonic_HC_SR04(trigPin, echoPin);
    }


    
    void setup() override
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
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
    WaterLevelRecipentType recipentType;
    int diameterTopCM;
    int diameterBaseCM;
    int heightCM;
    int distanceToSensorCM;
    int trigPin;
    int echoPin;
    float actualHeightCM = 0;

    unsigned long lastReadTime = 0;
    unsigned long readIntervalMs = 0; 

    float levelPercent = 0;
    float levelLiters = 0;
    float lastDistanceCm = 0;
    SensorUltrassonic_HC_SR04 *sr04SensorUltrassonicCapability;

    float calcularVolumeAgua(float alturaAtualCm)
    {
        if (alturaAtualCm <= 0.0)
            return 0.0;

        if (alturaAtualCm > heightCM)
            alturaAtualCm = heightCM;

        float hTotal = heightCM;
        float rTopo = diameterTopCM / 2.0;
        float rBase = diameterBaseCM / 2.0;

        float volumeTotalLitros = 0.0;

        for (int i = 0; i < alturaAtualCm; i++)
        {
            float h1 = i;
            float h2 = i + 1;

            
            float r1 = rTopo - (rTopo - rBase) * (h1 / hTotal);
            float r2 = rTopo - (rTopo - rBase) * (h2 / hTotal);

            
            float v_cm3 = (1.0 / 3.0) * M_PI * 1.0 * (r1 * r1 + r1 * r2 + r2 * r2);
            volumeTotalLitros += v_cm3 / 1000.0; 
        }

        return volumeTotalLitros;
    }

    float convertDistanceSensorToHeightVolume(float distanceCm)
    {
        return (distanceCm - distanceToSensorCM);
    }

    void handleSensor() override
    {
        sr04SensorUltrassonicCapability->measureDistance();
        float distanceCm = sr04SensorUltrassonicCapability->getDistanceCm();

        if (distanceCm < distanceToSensorCM)
            return;
            
        
        if (lastDistanceCm > 0) 
        {
            float variation = abs(distanceCm - lastDistanceCm) / lastDistanceCm * 100;
            if (variation > 10.0)
            {
                return; 
            }
        }


        lastDistanceCm = distanceCm;
        float heightWithoutWater = convertDistanceSensorToHeightVolume(distanceCm);
        float actualRealHeight = heightCM - heightWithoutWater;

        float actualLevelLiters = calcularVolumeAgua(actualRealHeight);

        if (actualLevelLiters == levelLiters)
            return;

        if (actualLevelLiters < levelLiters)
        {
            float diff = levelLiters - actualLevelLiters;
            if (diff > 20 && diff < 24)
            {
                return;
            }
        }

        actualHeightCM = actualRealHeight;
        levelLiters = actualLevelLiters;
        levelPercent = (actualRealHeight / heightCM) * 100;
    }
};
