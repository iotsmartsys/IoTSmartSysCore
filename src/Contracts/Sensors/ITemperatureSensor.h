#pragma once

#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    class ITemperatureSensor : public IHardwareAdapter
    {
    public:
        ITemperatureSensor() = default;
        virtual ~ITemperatureSensor() = default;
        virtual void setup() = 0;
        virtual float readTemperatureCelsius() = 0;
    };

}