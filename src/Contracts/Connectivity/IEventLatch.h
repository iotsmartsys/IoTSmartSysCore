#pragma once
#include <cstdint>

namespace iotsmartsys::core
{

    class IEventLatch
    {
    public:
        virtual ~IEventLatch() = default;

        virtual void set(uint32_t bits) = 0;
        virtual void clear(uint32_t bits) = 0;
        virtual uint32_t get() const = 0;

        // Retorna true quando TODOS os requiredBits estiverem setados.
        // timeoutMs: -1 = espera infinita
        virtual bool waitAll(uint32_t requiredBits, int32_t timeoutMs) = 0;
    };

} // namespace iotsmartsys::core