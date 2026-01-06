#pragma once
#include <Arduino.h>

#include "Contracts/Sensors/IGlpMeter.h"
#include "Contracts/Logging/ILogger.h"

#ifdef HX711_ENABLED
#include "HX711.h"
#endif

namespace iotsmartsys::platform::arduino
{
    using namespace iotsmartsys::core;
    using namespace iotsmartsys::core;

#ifdef HX711_ENABLED
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
        long lastStateReadMillis() const override;

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
        long lastStateReadMillis_{0};
    };
#else
    // Stub when HX711 is disabled at build time.
    class ArduinoGlpMeter : public IGlpMeter
    {
    public:
        ArduinoGlpMeter(int, ILogger &) {}
        void setup() override {}
        void handle() override {}
        float getKg() const override { return 0.0f; }
        float getPercent() const override { return 0.0f; }
        std::string getLevelString() const override { return "0"; }
        long lastStateReadMillis() const override { return 0; }
    };
#endif
} // namespace iotsmartsys::platform::arduino
