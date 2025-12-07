#include "DistanceSensorCapability.h"
#include "Utils/Logger.h"

DistanceSensorCapability::DistanceSensorCapability(int trigPin, int echoPin, SensorModel sensorModel)
    : Capability("", DISTANCE_SENSOR_TYPE, "0"), trigPin(trigPin), echoPin(echoPin), sensorModel(sensorModel), lastDistance(0), distanceCm(0), distanceInch(0), minDistance(20), maxDistance(400), sr04SensorUltrassonicCapability(nullptr)
{
    setCallback([this](String *state)
                {
                    LOG_PRINTLN("");
                    LOG_PRINTLN("Callback da " + this->capability_name + " chamado com o estado: " + *state);
                });
}

void DistanceSensorCapability::setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    switch (sensorModel)
    {
    case SensorModel::HC_SR04:
        sr04SensorUltrassonicCapability = new SensorUltrassonic_HC_SR04(trigPin, echoPin, minDistance, maxDistance);
        break;
    default:
        break;
    }
}

void DistanceSensorCapability::handle()
{
    handleSensor();
    if (lastDistance != distanceCm)
    {
        lastDistance = distanceCm;
        updateState(String(distanceCm));
    }
}

float DistanceSensorCapability::getDistanceCm()
{
    return distanceCm;
}

float DistanceSensorCapability::getDistanceInch()
{
    return distanceInch;
}

void DistanceSensorCapability::setMinDistance(long minDistance)
{
    this->minDistance = minDistance;
}

void DistanceSensorCapability::setMaxDistance(long maxDistance)
{
    this->maxDistance = maxDistance;
}

void DistanceSensorCapability::handleSensor()
{
    LOG_PRINTLN("Medindo distância");
    switch (sensorModel)
    {
    case SensorModel::HC_SR04:
        LOG_PRINTLN("Sensor HC-SR04");
        sr04SensorUltrassonicCapability->measureDistance();
        distanceCm = sr04SensorUltrassonicCapability->getDistanceCm();
        LOG_PRINTLN("Distância em cm: " + String(distanceCm));
        break;
    default:
        LOG_PRINTLN("Sensor não reconhecido");
        break;
    }
}
