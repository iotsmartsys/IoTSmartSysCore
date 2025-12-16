#pragma once

#include "ICapability.h"
#include "Contracts/Sensors/IColorSensor.h"
#include <string>

namespace iotsmartsys::core
{
    class OperationalColorSensorCapability : public ICapability
    {
    public:
        explicit OperationalColorSensorCapability(IColorSensor *sensor, unsigned long readIntervalMs = 60000);

        void setup() override;
        void handle() override;

    private:
        IColorSensor *sensor{nullptr};
        std::string lastState;
        unsigned long lastCheckMillis{0};
        unsigned long readIntervalMs{60000};
    };

} // namespace iotsmartsys::core
