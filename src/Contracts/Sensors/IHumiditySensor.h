#pragma once
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    class IHumiditySensor : public IHardwareAdapter
    {
    public:
        IHumiditySensor() = default;
        virtual ~IHumiditySensor() = default;
        virtual void setup() = 0;
        virtual float getHumidityPercentage() = 0;
    };

} // namespace iotsmartsys::core