
#pragma once

#include <cstdint>

namespace iotsmartsys::core
{

    struct ITimeProvider
    {
        virtual ~ITimeProvider() = default;

        // Tempo em milissegundos, monotônico (não precisa ser "epoch"):
        virtual std::uint64_t nowMs() const = 0;
    };

} // namespace iotsmartsys::core