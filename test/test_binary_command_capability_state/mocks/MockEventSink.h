#pragma once

#include <vector>
#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Events/CapabilityStateChanged.h"

namespace iotsmartsys::test::mocks
{

    class MockEventSink : public iotsmartsys::core::ICapabilityEventSink
    {
    public:
        std::vector<iotsmartsys::core::CapabilityStateChanged> events;

        void onStateChanged(const iotsmartsys::core::CapabilityStateChanged &ev) override
        {
            events.push_back(ev);
        }

        void clear() { events.clear(); }
    };

} // namespace iotsmartsys::test::mocks
