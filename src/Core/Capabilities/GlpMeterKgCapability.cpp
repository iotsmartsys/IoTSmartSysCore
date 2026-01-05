#include "Contracts/Capabilities/GlpMeterKgCapability.h"
#include "Contracts/Providers/Time.h"
#include <string>

using namespace iotsmartsys::core;

GlpMeterKgCapability::GlpMeterKgCapability(IGlpMeter &meter, ICapabilityEventSink *event_sink)
    : ICapability(event_sink, "", GLP_METER_TYPE, "0"), meter(meter)
{
}

void GlpMeterKgCapability::setup()
{
    meter.setup();
}

void GlpMeterKgCapability::handle()
{
    meter.handle();

    unsigned long now = timeProvider.nowMs();
    if (now - lastCheckMillis >= 1000 || lastKg == 0.0f)
    {
        lastCheckMillis = now;
        meter.handle();

        float kg = meter.getKg();

        // build state string (kg with 2 decimals)
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "%.2f", kg);
        std::string state = n > 0 ? std::string(buf) : std::string();

        if (kg != lastKg)
        {
            lastKg = kg;
            updateState(state);
        }
    }
}

float GlpMeterKgCapability::getKg() const
{
    return lastKg;
}
