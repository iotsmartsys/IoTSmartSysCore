#pragma once
#include <cstdint>

namespace iotsmartsys::core
{

    enum class RoutineResult : uint8_t
    {
        // executou e pode esperar o intervalo normal
        Ok,
        // falha transitória; retry com backoff curto
        RetrySoon,
        // falha; retry com backoff maior
        RetryLater
    };

    class IGatedRoutine
    {
    public:
        virtual ~IGatedRoutine() = default;

        virtual const char *name() const = 0;

        virtual uint32_t requiredBits() const = 0;

        virtual uint32_t intervalMs() const = 0;

        virtual RoutineResult execute() = 0;
    };

} // namespace iotsmartsys::core