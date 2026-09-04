#pragma once

#include <Adafruit_INA3221.h>
#include <Wire.h>

#include <cstdint>

namespace iotsmartsys::platform::arduino
{
    enum class INA3221Averaging
    {
        SAMPLES_1,
        SAMPLES_4,
        SAMPLES_16,
        SAMPLES_64,
        SAMPLES_128,
        SAMPLES_256,
        SAMPLES_512,
        SAMPLES_1024
    };

    enum class INA3221ConversionTime
    {
        US_140,
        US_204,
        US_332,
        US_588,
        MS_1,
        MS_2,
        MS_4,
        MS_8
    };

    struct INA3221DeviceConfig
    {
        std::uint8_t i2cAddress{0x40};
        INA3221Averaging averaging{INA3221Averaging::SAMPLES_16};
        INA3221ConversionTime busConversionTime{INA3221ConversionTime::MS_1};
        INA3221ConversionTime shuntConversionTime{INA3221ConversionTime::MS_1};

    public:
        static INA3221DeviceConfig createDeviceConfig()
        {
            INA3221DeviceConfig config;
            config.i2cAddress = 0x40;
            config.averaging = INA3221Averaging::SAMPLES_16;
            config.busConversionTime = INA3221ConversionTime::MS_1;
            config.shuntConversionTime = INA3221ConversionTime::MS_1;
            return config;
        }
    };

    class INA3221Device final
    {
    public:
        explicit INA3221Device(TwoWire &wire, const INA3221DeviceConfig &config = {});

        bool setup();
        bool available() const;
        bool configureShuntResistance(std::uint8_t channel, float resistanceOhms);
        float busVoltage(std::uint8_t channel);
        float currentAmps(std::uint8_t channel);

    private:
        TwoWire &_wire;
        INA3221DeviceConfig _config;
        Adafruit_INA3221 _driver;
        float _shuntResistanceOhms[3]{};
        bool _shuntConfigured[3]{};
        bool _setupAttempted{false};
        bool _available{false};
    };
}
