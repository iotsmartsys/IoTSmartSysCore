#pragma once

#include "Contracts/Providers/IRandomProvider.h"

namespace iotsmartsys::platform::espressif
{
    class EspressifRandomProvider final : public iotsmartsys::core::IRandomProvider
    {
    public:
        uint32_t nextU32() override;
    };
} // namespace iotsmartsys::platform::espressif
