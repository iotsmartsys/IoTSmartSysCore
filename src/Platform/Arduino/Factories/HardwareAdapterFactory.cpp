#include "Arduino.h"

#include "ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"
#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"

namespace iotsmartsys::platform::arduino
{
    std::size_t ArduinoHardwareAdapterFactory::relayAdapterSize() const
    {
        return sizeof(RelayHardwareAdapter);
    }

    std::size_t ArduinoHardwareAdapterFactory::relayAdapterAlign() const
    {
        return alignof(RelayHardwareAdapter);
    }

    iotsmartsys::core::IHardwareAdapter *ArduinoHardwareAdapterFactory::createRelay(
        void *mem,
        std::uint8_t pin,
        bool highIsOn)
    {
        const auto logic = highIsOn
                               ? HardwareDigitalLogic::HIGH_IS_ON
                               : HardwareDigitalLogic::LOW_IS_ON;

        // ✅ sem heap, sem fragmentação
        return new (mem) RelayHardwareAdapter(pin, logic);
    }

} // namespace iotsmartsys::platform::arduino