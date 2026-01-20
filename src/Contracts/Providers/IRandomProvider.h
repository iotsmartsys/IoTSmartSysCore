#pragma once

#include <cstdint>
#include <limits>

namespace iotsmartsys::core
{
    class IRandomProvider
    {
    public:
        virtual ~IRandomProvider() = default;
        virtual uint32_t nextU32() = 0;

        uint32_t uniform(uint32_t maxInclusive)
        {
            if (maxInclusive == 0)
                return 0;
            if (maxInclusive == std::numeric_limits<uint32_t>::max())
                return nextU32();
            return nextU32() % (maxInclusive + 1U);
        }
    };
} // namespace iotsmartsys::core
