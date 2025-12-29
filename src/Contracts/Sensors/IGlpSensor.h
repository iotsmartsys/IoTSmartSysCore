#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    class IGlpSensor : public IHardwareAdapter
    {
    public:
        IGlpSensor() = default;
        virtual ~IGlpSensor() = default;

        virtual void setup() = 0;
        virtual void handleSensor() = 0;
        virtual float getLevelPercent() = 0;
        virtual bool isDetected() = 0;
        virtual std::string getLevelString() = 0;
    };

} // namespace iotsmartsys::core
