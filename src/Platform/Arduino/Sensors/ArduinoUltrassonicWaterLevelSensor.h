#pragma once
#include <Arduino.h>
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/WaterLevelRecipentType.h"
#include "SensorUltrassonic_HC_SR04.h"

namespace iotsmartsys::platform::arduino
{
    class ArduinoUltrassonicWaterLevelSensor : public core::IWaterLevelSensor
    {
    public:
        ArduinoUltrassonicWaterLevelSensor(SensorUltrassonic_HC_SR04 *sr04Sensor, iotsmartsys::core::WaterLevelRecipentType recipentType, unsigned long intervalMs = 0);

        void setup() override;
        void handleSensor() override;

        float getLevelPercent() override;
        float getLevelLiters() override;
        float getHeightWaterInCm() override;

    // implement IHardwareAdapter contract (sensors typically don't support commands)
    bool applyCommand(const core::IHardwareCommand &command) override;
    bool applyCommand(const std::string &value) override;
    std::string getState() override;

    private:
        iotsmartsys::core::WaterLevelRecipentType recipentType;
        int diameterTopCM;
        int diameterBaseCM;
        int heightCM;
        int distanceToSensorCM;
        SensorUltrassonic_HC_SR04 *sr04Sensor;
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