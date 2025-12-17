#include "Contracts/Capabilities/GlpMeterCapability.h"
#include "Contracts/Providers/Time.h"
#include <string>

using namespace iotsmartsys::core;

GlpMeterCapability::GlpMeterCapability(IGlpMeter *meter, ICapabilityEventSink *event_sink)
    : ICapability(event_sink, GLP_METER_TYPE, "glp_meter", "0"), meter(meter)
{
}

void GlpMeterCapability::setup()
{
    if (meter)
    {
        meter->setup();
    }
}

void GlpMeterCapability::handle()
{
    if (!meter)
        return;

    unsigned long now = timeProvider.nowMs();
    if (now - lastCheckMillis >= 1000 || lastState.empty())
    {
        lastCheckMillis = now;
        meter->handle();

        float kg = meter->getKg();
        float percent = meter->getPercent();

        // build state string (percent with 2 decimals)
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "%.2f", percent);
        std::string state = n > 0 ? std::string(buf) : std::string();

        if (state != lastState)
        {
            lastState = state;
            lastKg = kg;
            lastPercent = percent;
            updateState(state);
        }
    }
}

float GlpMeterCapability::getKg() const
{
    return lastKg;
}

float GlpMeterCapability::getPercent() const
{
    return lastPercent;
}
