#pragma once

#include "Contracts/Providers/IRandomProvider.h"

namespace iotsmartsys::platform::arduino
{
    class Esp8266RandomProvider final : public iotsmartsys::core::IRandomProvider
    {
    public:
        uint32_t nextU32() override;
    };
} // namespace iotsmartsys::platform::arduino
