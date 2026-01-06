#pragma once

#include <Arduino.h>
#include "Contracts/Sensors/IGlpSensor.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::platform::arduino
{
    using namespace iotsmartsys::core;

    class ArduinoGlpSensor : public IGlpSensor
    {
    public:
        ArduinoGlpSensor(int pinAO, int pinDO, ILogger &logger);
        virtual ~ArduinoGlpSensor() = default;
        void setup() override;
        void handle() override;
        float getLevelPercent() override;
        bool isDetected() override;
        std::string getLevelString() override;
        long lastStateReadMillis() const override;

    private:
        ILogger &_logger;
        int pinAO;
        int pinDO;
        bool detected;
        std::string lastLevel;
        std::string levelState = GlpSensorLevelStrings::UNDETECTED;
        float levelPercent;
        const int NUM_SAMPLES = 20;
        int cleanAirBase = 0;
        bool calibrated = false;
        float lastReportedPercent = -1.0f;
        bool lastReportedDetected = false;
        long lastStateReadMillis_{0};

        int readAnalogAverage();
    };
} // namespace iotsmartsys::platform::arduino
