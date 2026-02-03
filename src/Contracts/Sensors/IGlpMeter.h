#pragma once

#include <string>
#include "Contracts/Adapters/IHardwareAdapter.h"

namespace iotsmartsys::core
{
    class IGlpMeter : public IHardwareAdapter
    {
    public:
        IGlpMeter() = default;
        virtual ~IGlpMeter() = default;

        virtual void setup() = 0;
        virtual void handle() = 0;

        // returns last stable kilograms measured
        virtual float getKg() const = 0;
    };

} // namespace iotsmartsys::core
