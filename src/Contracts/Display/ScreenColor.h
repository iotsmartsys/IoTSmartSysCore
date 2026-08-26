#pragma once

#include <cstdint>

namespace iotsmartsys::core
{
    enum class ScreenColor : std::uint8_t
    {
        Default = 0,
        White,
        Red,
        Green,
        Blue,
        Yellow,
        Cyan,
        Magenta
    };
} // namespace iotsmartsys::core
