#include "Contracts/Capabilities/LuminosityCapability.h"

namespace iotsmartsys::core
{
    LuminosityCapability::LuminosityCapability(const std::string &name,
                                               ILuminositySensor &sensor,
                                               ICapabilityEventSink *event_sink,
                                               float variationTolerance,
                                               float readIntervalMs)
        : PollingFloatCapability(event_sink, name.c_str(), LIGHT_SENSOR_TYPE, "-1", static_cast<unsigned long>(readIntervalMs), variationTolerance, 2),
          _sensor(sensor)
    {
    }

    LuminosityCapability::LuminosityCapability(ILuminositySensor &sensor,
                                               ICapabilityEventSink *event_sink,
                                               float variationTolerance,
                                               float readIntervalMs)
        : PollingFloatCapability(event_sink, "", LIGHT_SENSOR_TYPE, "-1", static_cast<unsigned long>(readIntervalMs), variationTolerance, 2),
          _sensor(sensor)
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
        if (!shouldRead(currentTime))
        {
            return;
        }

        const float lux = _sensor.readLux();
        publishIfChanged(lux);
    }

    float LuminosityCapability::getLux()
    {
        return lastValue();
    }

} // namespace iotsmartsys::core
