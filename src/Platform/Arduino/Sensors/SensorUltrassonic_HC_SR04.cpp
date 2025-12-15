#pragma once

#include <Arduino.h>
#include "SensorUltrassonic_HC_SR04.h"

namespace iotsmartsys::platform::arduino
{

#define SENSORULTRASSONIC_HC_SR04_TIMEOUT 30000
#define SENSORULTRASSONIC_HC_SR04_SOUND_SPEED 0.0343
#define TIME_TOLERANCE_MEASURE 3 * 1000
#define MIN_INTERVAL_BETWEEN_SMALLER_READINGS 500

    SensorUltrassonic_HC_SR04::SensorUltrassonic_HC_SR04(int trigPin, int echoPin)
    {
        this->trigPin = trigPin;
        this->echoPin = echoPin;
    }

    SensorUltrassonic_HC_SR04::SensorUltrassonic_HC_SR04(int trigPin, int echoPin, long minDistance, long maxDistance)
        : SensorUltrassonic_HC_SR04(trigPin, echoPin)
    {
        this->minDistance = minDistance;
        this->maxDistance = maxDistance;
    }

    void SensorUltrassonic_HC_SR04::setup()
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    float SensorUltrassonic_HC_SR04::getDistanceCm() const
    {
        return distanceCm;
    }

    void SensorUltrassonic_HC_SR04::measureDistance()
    {
        unsigned long currentTime = millis();
        if (currentTime - lastMeasurementTime < TIME_TOLERANCE_MEASURE)
        {
            return;
        }

        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        duration = pulseIn(echoPin, HIGH, SENSORULTRASSONIC_HC_SR04_TIMEOUT);
        float distanceCmCurrentRead = duration * SENSORULTRASSONIC_HC_SR04_SOUND_SPEED / 2;
        if (distanceCmCurrentRead <= 30)
        {
            if (currentTime - lastRejectedSmallerTime < MIN_INTERVAL_BETWEEN_SMALLER_READINGS)
            {
                return;
            }
            lastRejectedSmallerTime = currentTime;
            return;
        }

        if (distanceCmCurrentRead >= minDistance && distanceCmCurrentRead <= maxDistance)
        {
            lastReadings[readingIndex++] = distanceCmCurrentRead;
            if (readingIndex >= BUFFER_SIZE)
            {
                readingIndex = 0;
                bufferFilled = true;
            }

            float distanceCmAverage = calculateAverage();

            if (distanceCm == 0 || abs(distanceCmAverage - distanceCm) >= 2)
            {
                distanceCm = distanceCmAverage;
                lastMeasurementTime = currentTime;
            }
        }
    }

    float SensorUltrassonic_HC_SR04::calculateAverage()
    {
        int count = bufferFilled ? BUFFER_SIZE : readingIndex;
        float sum = 0;
        for (int i = 0; i < count; i++)
        {
            sum += lastReadings[i];
        }
        return count > 0 ? sum / count : 0;
    }
};