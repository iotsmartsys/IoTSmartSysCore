#include "Contracts/Capabilities/WaterLevelPercentCapability.h"
#include <sstream>
#include <iomanip>

namespace iotsmartsys::core
{
    WaterLevelPercentCapability::WaterLevelPercentCapability(IWaterLevelSensor *sensor)
        : ICapability(nullptr, WATER_LEVEL_PERCENT_SENSOR_TYPE, "0"), sensor(sensor), lastPercent(0.0f)
    {
    }

    void WaterLevelPercentCapability::setup()
    {
        ICapability::setup();
        if (sensor)
            sensor->setup();
    }

    void WaterLevelPercentCapability::handle()
    {
        if (!sensor)
            return;

        // sensor handles its own read interval; call its handle helper
        sensor->handle();

        float current = sensor->getLevelPercent();
        if (current != lastPercent)
        {
            lastPercent = current;
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(3) << current;
            updateState(ss.str());
        }
    }

    float WaterLevelPercentCapability::getLevelPercent() const
    {
        return lastPercent;
    }

} // namespace iotsmartsys::core
