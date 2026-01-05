#include "Contracts/Capabilities/WaterLevelPercentCapability.h"
#include <sstream>
#include <iomanip>

namespace iotsmartsys::core
{
    WaterLevelPercentCapability::WaterLevelPercentCapability(IWaterLevelSensor &sensor, ICapabilityEventSink *event_sink)
        : ICapability(event_sink, WATER_LEVEL_PERCENT_SENSOR_TYPE, "0"), sensor(sensor), lastPercent(0.0f)
    {
    }

    void WaterLevelPercentCapability::setup()
    {
    }

    void WaterLevelPercentCapability::handle()
    {
        sensor.handle();

        float current = sensor.getLevelPercent();
        if (current != lastPercent)
        {
            lastPercent = current;
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << current;
            updateState(ss.str());
        }
    }

    float WaterLevelPercentCapability::getLevelPercent() const
    {
        return lastPercent;
    }

} // namespace iotsmartsys::core
