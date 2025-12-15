#include "ArduinoUltrassonicWaterLevelSensor.h"
#include "SensorModel.h"

namespace iotsmartsys::platform::arduino
{
    ArduinoUltrassonicWaterLevelSensor::ArduinoUltrassonicWaterLevelSensor(SensorUltrassonic_HC_SR04 *sr04Sensor, iotsmartsys::core::WaterLevelRecipentType recipentType, unsigned long intervalMs) : core::IWaterLevelSensor()
    {
        this->sr04Sensor = sr04Sensor;
        this->recipentType = recipentType;
        this->readIntervalMs = intervalMs;

        switch (recipentType)
        {
        case iotsmartsys::core::WaterLevelRecipentType::Circle1000L:
            this->diameterTopCM = RECIPENT_1000L_DIAMETER_TOP;
            this->diameterBaseCM = RECIPENT_1000L_DIAMETER_BASE;
            this->heightCM = RECIPENT_1000L_HEIGHT;
            this->distanceToSensorCM = RECIPENT_1000L_DISTANCE_TO_SENSOR;
            break;
        case iotsmartsys::core::WaterLevelRecipentType::Circle2000L:
            this->diameterTopCM = RECIPENT_2000L_DIAMETER_TOP;
            this->diameterBaseCM = RECIPENT_2000L_DIAMETER_BASE;
            this->heightCM = RECIPENT_2000L_HEIGHT;
            this->distanceToSensorCM = RECIPENT_2000L_DISTANCE_TO_SENSOR;
            break;
        default:
            break;
        }
    }

    void ArduinoUltrassonicWaterLevelSensor::setup()
    {
        sr04Sensor->setup();
    }

    void ArduinoUltrassonicWaterLevelSensor::handleSensor()
    {
        sr04Sensor->measureDistance();
        float distanceCm = sr04Sensor->getDistanceCm();

        if (distanceCm < distanceToSensorCM)
            return;

        if (lastDistanceCm > 0)
        {
            float variation = abs(distanceCm - lastDistanceCm) / lastDistanceCm * 100;
            if (variation > 10.0)
            {
                return;
            }
        }

        lastDistanceCm = distanceCm;
        float heightWithoutWater = convertDistanceSensorToHeightVolume(distanceCm);
        float actualRealHeight = heightCM - heightWithoutWater;

        float actualLevelLiters = calcularVolumeAgua(actualRealHeight);

        if (actualLevelLiters == levelLiters)
            return;

        if (actualLevelLiters < levelLiters)
        {
            float diff = levelLiters - actualLevelLiters;
            if (diff > 20 && diff < 24)
            {
                return;
            }
        }

        actualHeightCM = actualRealHeight;
        levelLiters = actualLevelLiters;
        levelPercent = (actualRealHeight / heightCM) * 100;
    }

    float ArduinoUltrassonicWaterLevelSensor::getLevelPercent()
    {
        return levelPercent;
        return 0.0f;
    }

    float ArduinoUltrassonicWaterLevelSensor::getLevelLiters()
    {
        return levelLiters;
    }

    float ArduinoUltrassonicWaterLevelSensor::getHeightWaterInCm()
    {
        return actualHeightCM;
    }

    bool ArduinoUltrassonicWaterLevelSensor::applyCommand(const core::IHardwareCommand &command)
    {
        // sensors don't accept hardware commands; ignore
        (void)command;
        return false;
    }

    bool ArduinoUltrassonicWaterLevelSensor::applyCommand(const std::string &value)
    {
        (void)value;
        return false;
    }

    std::string ArduinoUltrassonicWaterLevelSensor::getState()
    {
        // return level percent as string with 3 decimals
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", levelPercent);
        return std::string(buf);
    }

    float ArduinoUltrassonicWaterLevelSensor::calcularVolumeAgua(float alturaAtualCm)
    {
        if (alturaAtualCm <= 0.0)
            return 0.0;

        if (alturaAtualCm > heightCM)
            alturaAtualCm = heightCM;

        float hTotal = heightCM;
        float rTopo = diameterTopCM / 2.0;
        float rBase = diameterBaseCM / 2.0;

        float volumeTotalLitros = 0.0;

        for (int i = 0; i < alturaAtualCm; i++)
        {
            float h1 = i;
            float h2 = i + 1;

            float r1 = rTopo - (rTopo - rBase) * (h1 / hTotal);
            float r2 = rTopo - (rTopo - rBase) * (h2 / hTotal);

            float v_cm3 = (1.0 / 3.0) * M_PI * 1.0 * (r1 * r1 + r1 * r2 + r2 * r2);
            volumeTotalLitros += v_cm3 / 1000.0;
        }

        return volumeTotalLitros;
    }

    float ArduinoUltrassonicWaterLevelSensor::convertDistanceSensorToHeightVolume(float distanceCm)
    {
        return (distanceCm - distanceToSensorCM);
    }

} // namespace iotsmartsys::platform::arduino