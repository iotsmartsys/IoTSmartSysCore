#include "Contracts/Capabilities/GlpMeterPercentCapability.h"
#include "Contracts/Providers/Time.h"
#include <string>

using namespace iotsmartsys::core;

GlpMeterPercentCapability::GlpMeterPercentCapability(IGlpMeter &meter, ICapabilityEventSink *event_sink)
    : ICapability(event_sink, "", GLP_METER_TYPE, "0"), meter(meter)
{
}

void GlpMeterPercentCapability::setup()
{
    meter.setup();
}

void GlpMeterPercentCapability::handle()
{
    meter.handle();

    unsigned long now = timeProvider.nowMs();
    if (now - lastCheckMillis >= 1000 || lastState.empty())
    {
        lastCheckMillis = now;
        meter.handle();

        float kg = meter.getKg();
        float percent = meter.getPercent();

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
