
#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include "Bh1750LuminositySensor.h"

namespace iotsmartsys::platform::arduino
{
    Bh1750LuminositySensor::Bh1750LuminositySensor(const int gpioSDA, const int gpioSCL) : _driver(), _gpioSDA(gpioSDA), _gpioSCL(gpioSCL)
    {
    }

    void Bh1750LuminositySensor::setup()
    {
        Serial.println("Initializing BH1750 sensor...");
        Wire.begin(_gpioSDA, _gpioSCL);
        // Configure in continuous high-res mode; capture success to avoid "not configured" errors.
        _initialized = _driver.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
        if (_initialized)
        {
            Serial.println("BH1750 sensor initialized.");
        }
        else
        {
            Serial.println("[BH1750] ERROR: failed to initialize sensor");
        }
    }

    float Bh1750LuminositySensor::readLux()
    {
        if (!_initialized)
        {
            Serial.println("[BH1750] Device not configured; attempting re-init...");
            _initialized = _driver.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
            if (!_initialized)
            {
                return -1.0f;
            }
        }

        return _driver.readLightLevel();
    }
} // namespace iotsmartsys::platform::arduino
