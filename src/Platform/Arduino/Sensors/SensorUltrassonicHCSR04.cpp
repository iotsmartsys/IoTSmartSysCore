
#include <Arduino.h>
#include <cmath>
#include "SensorUltrassonicHCSR04.h"

namespace iotsmartsys::platform::arduino
{
    SensorUltrassonicHCSR04::SensorUltrassonicHCSR04(SensorUltrassonicHCSR04Config cfg)
        : trigPin(cfg.triggerPin),
          echoPin(cfg.echoPin),
          minDistance(cfg.minDistance),
          maxDistance(cfg.maxDistance),
          timeOutMeasureUs(cfg.timeOutMeasureUs),
          soundSpeedCmPerUs(cfg.soundSpeedCmPerUs),
          timeToleranceMeasureMs(cfg.timeToleranceMeasureMs),
          distanceSuddenChangeThresholdCm(cfg.distanceSuddenChangeThresholdCm)
    {
        minDistanceCm = cfg.minDistance;
        maxDistanceCm = cfg.maxDistance;
    }

    void SensorUltrassonicHCSR04::setup()
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    void SensorUltrassonicHCSR04::handleSensor()
    {
        unsigned long currentTime = millis();
        if (lastMeasurementTime != 0 && currentTime - lastMeasurementTime < timeToleranceMeasureMs)
        {
            return;
        }
        lastMeasurementTime = currentTime;

        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        duration = pulseIn(echoPin, HIGH, timeOutMeasureUs);
        if (duration == 0)
        {
            return;
        }

        float distanceCmCurrentRead = duration * soundSpeedCmPerUs / 2;

        if (distanceCmCurrentRead >= minDistance && distanceCmCurrentRead <= maxDistance)
        {
            if (distanceCm > 0.0f && fabsf(distanceCmCurrentRead - distanceCm) >= distanceSuddenChangeThresholdCm)
            {
                resetReadings(distanceCmCurrentRead);
                this->distanceCm = distanceCmCurrentRead;
                actualDistanceCm = distanceCm;
                lastStateReadMillis_ = currentTime;
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
                actualDistanceCm = distanceCm;
                lastStateReadMillis_ = currentTime;
            }
        }
    }

    long SensorUltrassonicHCSR04::lastStateReadMillis() const
    {
        return lastStateReadMillis_;
    }

    bool SensorUltrassonicHCSR04::isLessOrEqualThan(float distanceCompare) const
    {
        return distanceCm > 0 && distanceCm <= distanceCompare;
    }

    bool SensorUltrassonicHCSR04::isGreaterOrEqualThan(float distanceCompare) const
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
