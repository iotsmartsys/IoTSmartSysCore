#pragma once

#include <Arduino.h>
#include "Capability.h"
#include "Sensors/Modules/SensorUltrassonic_HC_SR04.h"
#include "Sensors/Modules/SensorModel.h"


class DistanceSensorCapability : public Capability
{
public:
    DistanceSensorCapability(int trigPin, int echoPin, SensorModel sensorModel);

    void setup() override;
    void handle() override;

    float getDistanceCm();
    float getDistanceInch();

    void setMinDistance(long minDistance);
    void setMaxDistance(long maxDistance);

private:
    SensorModel sensorModel;
    int trigPin;
    int echoPin;
    float lastDistance;
    float distanceCm;
    float distanceInch;
    long minDistance;
    long maxDistance;
    SensorUltrassonic_HC_SR04 *sr04SensorUltrassonicCapability;

    void handleSensor();
};
