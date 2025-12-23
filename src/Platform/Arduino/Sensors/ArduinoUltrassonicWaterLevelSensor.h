#pragma once
#include <Arduino.h>
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/WaterLevelRecipentType.h"
#include "SensorUltrassonicHCSR04.h"

namespace iotsmartsys::platform::arduino
{
    class ArduinoUltrassonicWaterLevelSensor : public core::IWaterLevelSensor
    {
    public:
        ArduinoUltrassonicWaterLevelSensor(SensorUltrassonicHCSR04 *sr04Sensor, iotsmartsys::core::WaterLevelRecipentType recipentType, unsigned long intervalMs = 0);

        void setup() override;
        void handleSensor() override;

        float getLevelPercent() override;
        float getLevelLiters() override;
        float getHeightWaterInCm() override;

    // implement IHardwareAdapter contract (sensors typically don't support commands)
    bool applyCommand(const core::IHardwareCommand &command);
    bool applyCommand(const char *value);
    // Backwards-compatible overload
    bool applyCommand(const std::string &value) { return applyCommand(value.c_str()); }
    std::string getState();

    private:
        iotsmartsys::core::WaterLevelRecipentType recipentType;
        int diameterTopCM;
        int diameterBaseCM;
        int heightCM;
        int distanceToSensorCM;
        SensorUltrassonicHCSR04 *sr04Sensor;
        float actualHeightCM = 0;

        unsigned long lastReadTime = 0;
        unsigned long readIntervalMs = 0;

        float levelPercent = 0;
        float levelLiters = 0;
        float lastDistanceCm = 0;

        float calcularVolumeAgua(float alturaAtualCm);
        float convertDistanceSensorToHeightVolume(float distanceCm);
    };

} // namespace iotsmartsys::platform::arduino