// Platform/Arduino/Bh1750LuminositySensor.h
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include "Contracts/Core/Sensors/ILuminositySensor.h"

namespace iotsmartsys::platform::arduino
{
    class Bh1750LuminositySensor : public core::ILuminositySensor
    {
    public:
        explicit Bh1750LuminositySensor(BH1750 &driver)
            : _driver(driver)
        {
        }

        void begin()
        {
            Wire.begin();
            _driver.begin();
        }

        float readLux() override
        {
            return _driver.readLightLevel();
        }

    private:
        BH1750 &_driver;
    };

} // namespace iotsmartsys::platform::arduino