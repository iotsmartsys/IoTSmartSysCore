#include "Platform/Arduino/Sensors/INA3221Device.h"

#include <cmath>

#include "Contracts/Logging/Log.h"

namespace iotsmartsys::platform::arduino
{
    namespace
    {
        constexpr const char *kLogTag = "INA3221";

        bool mapAveraging(INA3221Averaging value, ina3221_avgmode &mapped)
        {
            switch (value)
            {
            case INA3221Averaging::SAMPLES_1: mapped = INA3221_AVG_1_SAMPLE; return true;
            case INA3221Averaging::SAMPLES_4: mapped = INA3221_AVG_4_SAMPLES; return true;
            case INA3221Averaging::SAMPLES_16: mapped = INA3221_AVG_16_SAMPLES; return true;
            case INA3221Averaging::SAMPLES_64: mapped = INA3221_AVG_64_SAMPLES; return true;
            case INA3221Averaging::SAMPLES_128: mapped = INA3221_AVG_128_SAMPLES; return true;
            case INA3221Averaging::SAMPLES_256: mapped = INA3221_AVG_256_SAMPLES; return true;
            case INA3221Averaging::SAMPLES_512: mapped = INA3221_AVG_512_SAMPLES; return true;
            case INA3221Averaging::SAMPLES_1024: mapped = INA3221_AVG_1024_SAMPLES; return true;
            }
            return false;
        }

        bool mapConversionTime(INA3221ConversionTime value, ina3221_convtime &mapped)
        {
            switch (value)
            {
            case INA3221ConversionTime::US_140: mapped = INA3221_CONVTIME_140US; return true;
            case INA3221ConversionTime::US_204: mapped = INA3221_CONVTIME_204US; return true;
            case INA3221ConversionTime::US_332: mapped = INA3221_CONVTIME_332US; return true;
            case INA3221ConversionTime::US_588: mapped = INA3221_CONVTIME_588US; return true;
            case INA3221ConversionTime::MS_1: mapped = INA3221_CONVTIME_1MS; return true;
            case INA3221ConversionTime::MS_2: mapped = INA3221_CONVTIME_2MS; return true;
            case INA3221ConversionTime::MS_4: mapped = INA3221_CONVTIME_4MS; return true;
            case INA3221ConversionTime::MS_8: mapped = INA3221_CONVTIME_8MS; return true;
            }
            return false;
        }
    }

    INA3221Device::INA3221Device(TwoWire &wire, const INA3221DeviceConfig &config)
        : _wire(wire), _config(config)
    {
    }

    bool INA3221Device::setup()
    {
        if (_setupAttempted)
        {
            return _available;
        }
        _setupAttempted = true;

        ina3221_avgmode averaging;
        ina3221_convtime busConversionTime;
        ina3221_convtime shuntConversionTime;
        if (_config.i2cAddress < 0x40 || _config.i2cAddress > 0x43 ||
            !mapAveraging(_config.averaging, averaging) ||
            !mapConversionTime(_config.busConversionTime, busConversionTime) ||
            !mapConversionTime(_config.shuntConversionTime, shuntConversionTime))
        {
            iotsmartsys::core::Log::get().error(
                kLogTag, "Invalid device configuration at address 0x%02X.",
                static_cast<unsigned>(_config.i2cAddress));
            return false;
        }

        if (!_driver.begin(_config.i2cAddress, &_wire))
        {
            iotsmartsys::core::Log::get().error(
                kLogTag, "Device initialization failed at address 0x%02X.",
                static_cast<unsigned>(_config.i2cAddress));
            return false;
        }

        if (!_driver.setAveragingMode(averaging) ||
            !_driver.setBusVoltageConvTime(busConversionTime) ||
            !_driver.setShuntVoltageConvTime(shuntConversionTime) ||
            !_driver.setMode(INA3221_MODE_SHUNT_BUS_CONT))
        {
            iotsmartsys::core::Log::get().error(
                kLogTag, "Mandatory configuration failed at address 0x%02X.",
                static_cast<unsigned>(_config.i2cAddress));
            return false;
        }

        _available = true;
        iotsmartsys::core::Log::get().info(
            kLogTag, "Device ready at address 0x%02X in continuous shunt and bus mode.",
            static_cast<unsigned>(_config.i2cAddress));
        return true;
    }

    bool INA3221Device::available() const
    {
        return _available;
    }

    bool INA3221Device::configureShuntResistance(std::uint8_t channel, float resistanceOhms)
    {
        if (!_available || channel > 2 || !std::isfinite(resistanceOhms) || resistanceOhms <= 0.0f)
        {
            return false;
        }

        if (_shuntConfigured[channel] && _shuntResistanceOhms[channel] != resistanceOhms)
        {
            iotsmartsys::core::Log::get().error(
                kLogTag, "Channel %u already uses a different shunt resistance.",
                static_cast<unsigned>(channel));
            return false;
        }

        _driver.setShuntResistance(channel, resistanceOhms);
        _shuntResistanceOhms[channel] = resistanceOhms;
        _shuntConfigured[channel] = true;
        return true;
    }

    float INA3221Device::busVoltage(std::uint8_t channel)
    {
        return _available ? _driver.getBusVoltage(channel) : NAN;
    }

    float INA3221Device::currentAmps(std::uint8_t channel)
    {
        return _available ? _driver.getCurrentAmps(channel) : NAN;
    }
}
