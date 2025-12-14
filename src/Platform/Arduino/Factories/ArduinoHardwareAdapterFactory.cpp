#include <Arduino.h>

#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"
#include "Platform/Arduino/Adapters/OutputHardwareAdapter.h"
#include "Platform/Arduino/Adapters/InputHardwareAdapter.h"

namespace iotsmartsys::platform::arduino
{
    /* Relay */
    std::size_t ArduinoHardwareAdapterFactory::relayAdapterSize() const
    {
        return sizeof(RelayHardwareAdapter);
    }

    std::size_t ArduinoHardwareAdapterFactory::relayAdapterAlign() const
    {
        return alignof(RelayHardwareAdapter);
    }

    static void destroyRelayAdapter(void *p)
    {
        if (!p) return;
        static_cast<RelayHardwareAdapter *>(p)->~RelayHardwareAdapter();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::relayAdapterDestructor() const
    {
        return &destroyRelayAdapter;
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

    /* Output */
    std::size_t ArduinoHardwareAdapterFactory::outputAdapterSize() const
    {
        return sizeof(OutputHardwareAdapter);
    }

    std::size_t ArduinoHardwareAdapterFactory::outputAdapterAlign() const
    {
        return alignof(OutputHardwareAdapter);
    }
    
    static void destroyOutputAdapter(void *p)
    {
        if (!p) return;
        static_cast<OutputHardwareAdapter *>(p)->~OutputHardwareAdapter();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::outputAdapterDestructor() const
    {
        return &destroyOutputAdapter;
    }
    
    iotsmartsys::core::IHardwareAdapter *ArduinoHardwareAdapterFactory::createOutput(
        void *mem,
        std::uint8_t pin,
        bool highIsOn)
    {
        const auto logic = highIsOn
                               ? HardwareDigitalLogic::HIGH_IS_ON
                               : HardwareDigitalLogic::LOW_IS_ON;

        // ✅ sem heap, sem fragmentação
        return new (mem) OutputHardwareAdapter(pin, logic);
    }

    /* Input */
    std::size_t ArduinoHardwareAdapterFactory::inputAdapterSize() const
    {
        return sizeof(InputHardwareAdapter);
    }

    std::size_t ArduinoHardwareAdapterFactory::inputAdapterAlign() const
    {
        return alignof(InputHardwareAdapter);
    }

    static void destroyInputAdapter(void *p)
    {
        if (!p) return;
        static_cast<InputHardwareAdapter *>(p)->~InputHardwareAdapter();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::inputAdapterDestructor() const
    {
        return &destroyInputAdapter;
    }

    iotsmartsys::core::IHardwareAdapter *ArduinoHardwareAdapterFactory::createInput(
        void *mem,
        std::uint8_t pin)
    {
        // ✅ sem heap, sem fragmentação
        return new (mem) InputHardwareAdapter(pin);
    }

} // namespace iotsmartsys::platform::arduino