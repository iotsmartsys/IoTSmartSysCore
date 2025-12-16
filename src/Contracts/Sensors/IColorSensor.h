#pragma once

#include <string>
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    class IColorSensor : public IHardwareAdapter
    {
    public:
        IColorSensor() = default;
        virtual ~IColorSensor() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;
        virtual std::string getStateString() const = 0;
    };

} // namespace iotsmartsys::core
