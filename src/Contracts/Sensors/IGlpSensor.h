#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    struct GlpSensorLevelStrings
    {
        static constexpr const char *UNDETECTED = "UNDETECTED";
        static constexpr const char *GLP_SENSOR_LEVEL_LOW = "LOW";
        static constexpr const char *GLP_SENSOR_LEVEL_MEDIUM = "MEDIUM";
        static constexpr const char *GLP_SENSOR_LEVEL_HIGH = "HIGH";
    };

    class IGlpSensor : public IHardwareAdapter
    {
    public:
        IGlpSensor() = default;
        virtual ~IGlpSensor() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;

        /// @brief Lê o nível de GLP do sensor
        /// @return Nível de GLP em porcentagem (0.0 a 100.0)
        virtual float getLevelPercent() = 0;

        /// @brief Verifica se o sensor de GLP está ativado
        /// @return true se o sensor estiver ativado, false caso contrário
        virtual bool isDetected() = 0;

        /// @brief Obtém o nível de GLP do sensor como uma string
        /// @return Nível de GLP como string: "UNDETECTED", "LOW", "MEDIUM", "HIGH" (GlpSensorLevelStrings)
        virtual std::string getLevelString() = 0;
    };

} // namespace iotsmartsys::core