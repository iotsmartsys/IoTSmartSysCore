#include "Contracts/Capabilities/GlpMeterPercentCapability.h"
#include "Contracts/Providers/Time.h"
#include <string>

using namespace iotsmartsys::core;

GlpMeterPercentCapability::GlpMeterPercentCapability(IGlpMeter &meter, ICapabilityEventSink *event_sink, float maxKgExpected)
    : ICapability(event_sink, "", GLP_METER_TYPE, "0"), meter(meter), maxKgExpected(maxKgExpected)
{
}

void GlpMeterPercentCapability::setup()
{
    meter.setup();
}

void GlpMeterPercentCapability::handle()
{
    unsigned long now = timeProvider.nowMs();
    if (now - lastCheckMillis >= 1000 || lastState.empty())
    {
        lastCheckMillis = now;

        float kg = meter.getKg();
        float percent = 0.0f;
        if (maxKgExpected > 0.0f)
        {
            percent = (kg / maxKgExpected) * 100.0f;
            if (percent < 0.0f)
                percent = 0.0f;
            if (percent > 100.0f)
                percent = 100.0f;
        }

        // build state string (percent with 2 decimals)
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "%.2f", percent);
        std::string state = n > 0 ? std::string(buf) : std::string();

        if (state != lastState)
        {
            lastState = state;
            lastPercent = percent;
            updateState(state);
        }
    }
}

float GlpMeterPercentCapability::getPercent() const
{
    return lastPercent;
}
