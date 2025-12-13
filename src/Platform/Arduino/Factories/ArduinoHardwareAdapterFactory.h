#pragma once

#include <cstddef>
#include <cstdint>

#include "Contracts/Core/Adapters/IHardwareAdapterFactory.h"

namespace iotsmartsys::platform::arduino
{

    class ArduinoHardwareAdapterFactory final : public iotsmartsys::core::IHardwareAdapterFactory
    {
    public:
        ArduinoHardwareAdapterFactory() = default;
        ~ArduinoHardwareAdapterFactory() override = default;

        std::size_t relayAdapterSize() const override;
        std::size_t relayAdapterAlign() const override;

        iotsmartsys::core::IHardwareAdapter *createRelay(void *mem, std::uint8_t pin, bool highIsOn) override;
    };

} // namespace iotsmartsys::platform::arduino