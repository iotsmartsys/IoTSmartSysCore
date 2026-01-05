#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "Contracts/Sensors/ILuminositySensor.h"

#ifdef BH1750_ENABLED
#include <BH1750.h>
#endif

namespace iotsmartsys::platform::arduino
{
#ifdef BH1750_ENABLED
    class Bh1750LuminositySensor : public core::ILuminositySensor
    {
    public:
        Bh1750LuminositySensor(const int gpioSDA, const int gpioSCL);

        void setup();

        float readLux() override;

    private:
        BH1750 _driver;
        int _gpioSDA;
        int _gpioSCL;
        bool _initialized{false};
    };
#else
    // Stub used when BH1750 support is disabled at build time.
    class Bh1750LuminositySensor : public core::ILuminositySensor
    {
    public:
        Bh1750LuminositySensor(const int, const int) {}
        void setup() {}
        float readLux() override { return -1.0f; }
    };
#endif

} // namespace iotsmartsys::platform::arduino
