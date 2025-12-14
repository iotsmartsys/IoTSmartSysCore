#include "Contracts/Capabilities/LuminosityCapability.h"

namespace iotsmartsys::core
{
    LuminosityCapability::LuminosityCapability(const std::string &name,
                                               ILuminositySensor &sensor,
                                               core::ITimeProvider &timeProvider,
                                               float variationTolerance,
                                               float readIntervalSeconds)
        : ICapability(LIGHT_SENSOR_TYPE, "-1"),
          _sensor(sensor),
          _variationTolerance(variationTolerance),
          _timeProvider(timeProvider),
          _readIntervalMs(static_cast<unsigned long>(readIntervalSeconds * 1000.0f))
    {
    }

    void LuminosityCapability::setup()
    {
    }

    void LuminosityCapability::handle()
    {
        unsigned long currentTime = static_cast<unsigned long>(_timeProvider.nowMs());
        if (currentTime - _lastReadMs < _readIntervalMs)
        {
            return;
        }

        _lastReadMs = currentTime;

        float lux = _sensor.readLux();

        if (_lastLux != lux)
        {
            if (abs(_lastLux - lux) > _variationTolerance)
            {
                _lastLux = lux;
                updateState(std::to_string(_lastLux));
            }
        }
    }

    float LuminosityCapability::getLux()
    {
        return _lastLux;
    }

} // namespace iotsmartsys::core