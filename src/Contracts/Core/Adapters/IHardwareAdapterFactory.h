#pragma once
#include <stdint.h>
#include "Contracts/Core/Adapters/IHardwareAdapter.h"
#include <cstddef>

namespace iotsmartsys::core
{

    class IHardwareAdapterFactory
    {
    public:
        IHardwareAdapterFactory() = default;
        virtual ~IHardwareAdapterFactory() = default;

        IHardwareAdapterFactory(const IHardwareAdapterFactory &) = delete;
        IHardwareAdapterFactory &operator=(const IHardwareAdapterFactory &) = delete;

        virtual std::size_t relayAdapterSize() const = 0;
        virtual std::size_t relayAdapterAlign() const = 0;

        virtual IHardwareAdapter *createRelay(void *mem, std::uint8_t pin, bool highIsOn) = 0;
    };

} // namespace iotsmartsys::core