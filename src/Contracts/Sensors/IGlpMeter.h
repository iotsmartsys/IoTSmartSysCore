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

        // returns last stable percent (0..100)
        virtual float getPercent() const = 0;

        // human readable state string (e.g. "12.34") used by capability
        virtual std::string getLevelString() const = 0;
    };

} // namespace iotsmartsys::core
