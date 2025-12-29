#include "Contracts/Capabilities/LuminosityCapability.h"

namespace iotsmartsys::core
{
    LuminosityCapability::LuminosityCapability(const std::string &name,
                                               ILuminositySensor &sensor,
                                               ICapabilityEventSink *event_sink,
                                               float variationTolerance,
                                               float readIntervalMs)
        : ICapability(event_sink, LIGHT_SENSOR_TYPE, "-1"),
          _sensor(sensor),
          _variationTolerance(variationTolerance),
          _readIntervalMs(static_cast<unsigned long>(readIntervalMs))
    {
    }

    LuminosityCapability::LuminosityCapability(ILuminositySensor &sensor,
                                               ICapabilityEventSink *event_sink,
                                               float variationTolerance,
                                               float readIntervalMs)
        : ICapability(event_sink, LIGHT_SENSOR_TYPE, "-1"),
          _sensor(sensor),
          _variationTolerance(variationTolerance),
          _readIntervalMs(static_cast<unsigned long>(readIntervalMs))
    {
    }

    void LuminosityCapability::setup()
    {
        ICapability::setup();
        _sensor.setup();
    }

    void LuminosityCapability::handle()
    {
        unsigned long currentTime = static_cast<unsigned long>(timeProvider.nowMs());
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