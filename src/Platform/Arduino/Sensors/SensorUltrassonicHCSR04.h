#pragma once

#include <Arduino.h>

namespace iotsmartsys::platform::arduino
{

#define SENSORULTRASSONIC_HC_SR04_TIMEOUT 30000
#define SENSORULTRASSONIC_HC_SR04_SOUND_SPEED 0.0343
#define TIME_TOLERANCE_MEASURE 3 * 1000

    class SensorUltrassonicHCSR04
    {

    public:
        SensorUltrassonicHCSR04(int trigPin, int echoPin);
        SensorUltrassonicHCSR04(int trigPin, int echoPin, long minDistance, long maxDistance);

        void setup();

        float getDistanceCm() const;
        bool distanceIsLessOrEqualThan(float distanceCompare) const;
        bool distanceIsGreaterOrEqualThan(float distanceCompare) const;
        void measureDistance();

    private:
        int trigPin;
        int echoPin;
        unsigned long duration = 0;
        float distanceCm = 0.0f;
        unsigned long lastMeasurementTime = 0;
        long minDistance = 2;
        long maxDistance = 400;

        static const int BUFFER_SIZE = 5;
        float lastReadings[BUFFER_SIZE] = {0};
        int readingIndex = 0;
        bool bufferFilled = false;

        float calculateAverage();
        void resetReadings(float distanceCm);
    };

}
