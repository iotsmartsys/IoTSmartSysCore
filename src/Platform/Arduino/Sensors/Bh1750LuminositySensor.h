#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include "Contracts/Sensors/ILuminositySensor.h"

namespace iotsmartsys::platform::arduino
{
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

} // namespace iotsmartsys::platform::arduino
