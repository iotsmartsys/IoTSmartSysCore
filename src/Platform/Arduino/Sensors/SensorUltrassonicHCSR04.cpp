
#include <Arduino.h>
#include <cmath>
#include "SensorUltrassonicHCSR04.h"

namespace iotsmartsys::platform::arduino
{

#define SENSORULTRASSONIC_HC_SR04_TIMEOUT 30000
#define SENSORULTRASSONIC_HC_SR04_SOUND_SPEED 0.0343
#define TIME_TOLERANCE_MEASURE 3 * 1000
#define DISTANCE_SUDDEN_CHANGE_THRESHOLD_CM 10.0f

    SensorUltrassonicHCSR04::SensorUltrassonicHCSR04(int trigPin, int echoPin)
    {
        this->trigPin = trigPin;
        this->echoPin = echoPin;
    }

    SensorUltrassonicHCSR04::SensorUltrassonicHCSR04(int trigPin, int echoPin, long minDistance, long maxDistance)
        : SensorUltrassonicHCSR04(trigPin, echoPin)
    {
        this->minDistance = minDistance;
        this->maxDistance = maxDistance;
    }

    void SensorUltrassonicHCSR04::setup()
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    float SensorUltrassonicHCSR04::getDistanceCm() const
    {
        return distanceCm;
    }

    void SensorUltrassonicHCSR04::measureDistance()
    {
        unsigned long currentTime = millis();
        if (lastMeasurementTime != 0 && currentTime - lastMeasurementTime < TIME_TOLERANCE_MEASURE)
        {
            return;
        }
        lastMeasurementTime = currentTime;

        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        duration = pulseIn(echoPin, HIGH, SENSORULTRASSONIC_HC_SR04_TIMEOUT);
        if (duration == 0)
        {
            return;
        }

        float distanceCmCurrentRead = duration * SENSORULTRASSONIC_HC_SR04_SOUND_SPEED / 2;

        if (distanceCmCurrentRead >= minDistance && distanceCmCurrentRead <= maxDistance)
        {
            if (distanceCm > 0.0f && fabsf(distanceCmCurrentRead - distanceCm) >= DISTANCE_SUDDEN_CHANGE_THRESHOLD_CM)
            {
                resetReadings(distanceCmCurrentRead);
                this->distanceCm = distanceCmCurrentRead;
                return;
            }

            lastReadings[readingIndex++] = distanceCmCurrentRead;
            if (readingIndex >= BUFFER_SIZE)
            {
                readingIndex = 0;
                bufferFilled = true;
            }

            float distanceCmAverage = calculateAverage();

            if (distanceCm == 0 || fabsf(distanceCmAverage - distanceCm) >= 2.0f)
            {
                distanceCm = distanceCmAverage;
            }
        }
    }

    bool SensorUltrassonicHCSR04::distanceIsLessOrEqualThan(float distanceCompare) const
    {
        return distanceCm > 0 && distanceCm <= distanceCompare;
    }

    bool SensorUltrassonicHCSR04::distanceIsGreaterOrEqualThan(float distanceCompare) const
    {
        return distanceCm > 0 && distanceCm >= distanceCompare;
    }

    float SensorUltrassonicHCSR04::calculateAverage()
    {
        int count = bufferFilled ? BUFFER_SIZE : readingIndex;
        float sum = 0;
        for (int i = 0; i < count; i++)
        {
            sum += lastReadings[i];
        }
        return count > 0 ? sum / count : 0;
    }

    void SensorUltrassonicHCSR04::resetReadings(float distanceCm)
    {
        lastReadings[0] = distanceCm;
        readingIndex = 1;
        bufferFilled = false;
    }
};
