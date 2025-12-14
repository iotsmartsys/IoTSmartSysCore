#pragma once
#include <stdint.h>
#include "Contracts/Adapters/IHardwareAdapter.h"
#include <cstddef>

namespace iotsmartsys::core
{

    class IHardwareAdapterFactory
    {
    public:
        using AdapterDestructor = void (*)(void *);

        IHardwareAdapterFactory() = default;
        virtual ~IHardwareAdapterFactory() = default;

        IHardwareAdapterFactory(const IHardwareAdapterFactory &) = delete;
        IHardwareAdapterFactory &operator=(const IHardwareAdapterFactory &) = delete;

        virtual std::size_t relayAdapterSize() const = 0;
        virtual std::size_t relayAdapterAlign() const = 0;

        virtual IHardwareAdapter *createRelay(void *mem, std::uint8_t pin, bool highIsOn) = 0;
        virtual AdapterDestructor relayAdapterDestructor() const = 0;

        virtual std::size_t outputAdapterSize() const = 0;
        virtual std::size_t outputAdapterAlign() const = 0;
        virtual IHardwareAdapter *createOutput(void *mem, std::uint8_t pin, bool highIsOn) = 0;
        virtual AdapterDestructor outputAdapterDestructor() const = 0;

        virtual std::size_t inputAdapterSize() const = 0;
        virtual std::size_t inputAdapterAlign() const = 0;
        virtual IHardwareAdapter *createInput(void *mem, std::uint8_t pin) = 0;
        virtual AdapterDestructor inputAdapterDestructor() const = 0;
    };

} // namespace iotsmartsys::core