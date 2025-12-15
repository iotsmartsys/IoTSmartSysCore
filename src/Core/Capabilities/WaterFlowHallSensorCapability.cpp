#include "Contracts/Capabilities/WaterFlowHallSensorCapability.h"
#include <sstream>
#include <iomanip>

namespace iotsmartsys::core
{
    WaterFlowHallSensorCapability::WaterFlowHallSensorCapability(IInputHardwareAdapter *input_hardware_adapter)
        : IInputCapability(input_hardware_adapter, WATER_FLOW_SENSOR_TYPE, "0"),
          lastMillis(0),
          pulseCount(0),
          totalLiters(0.0f),
          lastTotalLiters(0.0f)
    {
    }

    void WaterFlowHallSensorCapability::setup()
    {
        ICapability::setup();
        lastMillis = static_cast<unsigned long>(timeProvider.nowMs());
    }

    void WaterFlowHallSensorCapability::handle()
    {
        unsigned long now = static_cast<unsigned long>(timeProvider.nowMs());
        if (now - lastMillis >= 1000)
        {
            // compute liters from pulses
            float liters = static_cast<float>(pulseCount) / 16274.3f;

            if (liters > 0.01f)
            {
                totalLiters += liters;
            }

            pulseCount = 0;
            lastMillis = now;

            if (totalLiters != lastTotalLiters)
            {
                lastTotalLiters = totalLiters;
                // format with 3 decimals
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(3) << totalLiters;
                updateState(ss.str());
            }
        }
    }

    void WaterFlowHallSensorCapability::incrementPulseCount()
    {
        ++pulseCount;
    }

} // namespace iotsmartsys::core
