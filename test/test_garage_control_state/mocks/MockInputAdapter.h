#pragma once

#include "Contracts/Adapters/IInputHardwareAdapter.h"

namespace iotsmartsys::test::mocks
{

    class MockInputAdapter : public iotsmartsys::core::IInputHardwareAdapter
    {
    public:
        int32_t state = HIGH;

        void setup() override {}
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }

        int32_t readInput() override { return state; }
        bool digitalActive() override { return state == LOW; }
        int32_t readDigitalState() override { return state; }

        void setState(int32_t newState) { state = newState; }
    };

} // namespace iotsmartsys::test::mocks
