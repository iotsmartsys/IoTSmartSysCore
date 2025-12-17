
#pragma once

#include "CapabilityStateChanged.h"

namespace iotsmartsys::core
{
    class ICapabilityEventSink
    {
    public:
        virtual ~ICapabilityEventSink() = default;

        virtual void onStateChanged(const CapabilityStateChanged &ev) = 0;
    };

} // namespace iotsmartsys::core