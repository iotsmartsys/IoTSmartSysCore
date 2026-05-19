#pragma once

#include "Contracts/Sensors/IDistanceSensor.h"

namespace iotsmartsys::platform::arduino
{

    struct SensorUltrassonicHCSR04Config : public core::DistanceSensorConfig
    {
        int triggerPin = -1;
        int echoPin = -1;
        long minDistance = 0;
        long maxDistance = 1000;
        long timeOutMeasureUs = 30000;
        float soundSpeedCmPerUs = 0.0343f;
        long timeToleranceMeasureMs = 3000;
        float distanceSuddenChangeThresholdCm = 10.0f;
    };

    class SensorUltrassonicHCSR04 : public core::IDistanceSensor
    {

    public:
        SensorUltrassonicHCSR04(SensorUltrassonicHCSR04Config cfg);

        void setup() override;
        void handleSensor() override;
        long lastStateReadMillis() const override;

        bool isLessOrEqualThan(float distanceCompare) const;
        bool isGreaterOrEqualThan(float distanceCompare) const;

    private:
        int trigPin;
        int echoPin;
        unsigned long duration = 0;
        float distanceCm = 0.0f;
        unsigned long lastMeasurementTime = 0;
        long minDistance = 2;
        long maxDistance = 400;
        long timeOutMeasureUs = 30000;
        float soundSpeedCmPerUs = 0.0343f;
        long timeToleranceMeasureMs = 3000;
        float distanceSuddenChangeThresholdCm = 10.0f;
        long lastStateReadMillis_ = 0;

        static const int BUFFER_SIZE = 5;
        float lastReadings[BUFFER_SIZE] = {0};
        int readingIndex = 0;
        bool bufferFilled = false;

        float calculateAverage();
        void resetReadings(float distanceCm);
    };

}
