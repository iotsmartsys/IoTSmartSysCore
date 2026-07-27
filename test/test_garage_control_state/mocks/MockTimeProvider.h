#pragma once

#include <cstdint>
#include "Contracts/Providers/ITimeProvider.h"

namespace iotsmartsys::test::mocks
{

    class MockTimeProvider : public iotsmartsys::core::ITimeProvider
    {
    public:
        mutable std::uint64_t currentMs = 0;

        std::uint64_t nowMs() const override { return currentMs; }

        void advance(std::uint64_t deltaMs) { currentMs += deltaMs; }
        void set(std::uint64_t ms) { currentMs = ms; }
    };

} // namespace iotsmartsys::test::mocks
