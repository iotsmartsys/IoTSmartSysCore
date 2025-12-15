#pragma once

#include <cstddef>
#include <cstdint>

#include "Contracts/Adapters/IHardwareAdapterFactory.h"

namespace iotsmartsys::platform::arduino
{

    class ArduinoHardwareAdapterFactory final : public iotsmartsys::core::IHardwareAdapterFactory
    {
    public:
        ArduinoHardwareAdapterFactory() = default;
        ~ArduinoHardwareAdapterFactory() override = default;

        /* Relay Adapter */
        std::size_t relayAdapterSize() const override;
        std::size_t relayAdapterAlign() const override;
        iotsmartsys::core::IHardwareAdapter *createRelay(void *mem, std::uint8_t pin, bool highIsOn) override;
        AdapterDestructor relayAdapterDestructor() const override;

        /* Output Adapter */
        std::size_t outputAdapterSize() const override;
        std::size_t outputAdapterAlign() const override;
        iotsmartsys::core::IHardwareAdapter *createOutput(void *mem, std::uint8_t pin, bool highIsOn) override;
        AdapterDestructor outputAdapterDestructor() const override;

        /* Input Adapter */
        std::size_t inputAdapterSize() const override;
        std::size_t inputAdapterAlign() const override;
        iotsmartsys::core::IHardwareAdapter *createInput(void *mem, std::uint8_t pin) override;
        AdapterDestructor inputAdapterDestructor() const override;


    };

} // namespace iotsmartsys::platform::arduino