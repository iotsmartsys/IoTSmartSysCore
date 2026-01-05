#pragma once
#include <Arduino.h>

#include "Contracts/Sensors/IGlpMeter.h"
#include "Contracts/Logging/ILogger.h"
#include "HX711.h"

namespace iotsmartsys::platform::arduino
{
    using namespace iotsmartsys::core;
    using namespace iotsmartsys::core;

    class ArduinoGlpMeter : public IGlpMeter
    {
    public:
        ArduinoGlpMeter(int pinAO, ILogger &logger);
        virtual ~ArduinoGlpMeter() = default;

        void setup() override;
        void handle() override;
        float getKg() const override;
        float getPercent() const override;
        std::string getLevelString() const override;

    private:
        ILogger &_logger;

        HX711 *scale;
        float actualPercent = 0.0;
        float lastPercent = 0.0;
        float actualKg = 0.0;
        float lastKg = 0.0;
        int dout_pin;
        int sck_pin;
        float tare_weight_kg;
        float weight_capacity_kg;
        std::string levelString;
    };
} // namespace iotsmartsys::platform::arduino