#pragma once

#include "Capability.h"

class GlpSensorCapability : public Capability
{
public:
    GlpSensorCapability(int pinAO, int pinDO);

    GlpSensorCapability(String capability_name, String description, String owner, String type, String mode, String value);

    void setup() override;
    void handle() override;

    /// @brief Verifica se o sensor de GLP está ativado
    /// @return true se o sensor estiver ativado, false caso contrário
    bool isDetected();

    /// @brief Lê o nível de GLP do sensor
    /// @return Nível de GLP em porcentagem (0.0 a 100.0)
    float getLevelPercent();
    
    String getLevelString();

private:
    int pinAO;
    int pinDO;
    bool detected;
    String lastLevel;
    String levelState = GLP_SENSOR_LEVEL_NONE;
    float levelPercent;

    int readAnalogAverage();
    // float handleSensorReading();
};
