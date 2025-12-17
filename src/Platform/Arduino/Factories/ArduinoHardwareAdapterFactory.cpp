#include <Arduino.h>

#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Adapters/RelayHardwareAdapter.h"
#include "Platform/Arduino/Adapters/OutputHardwareAdapter.h"
#include "Platform/Arduino/Adapters/InputHardwareAdapter.h"
#include "Platform/Arduino/Sensors/ArduinoUltrassonicWaterLevelSensor.h"

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
        if (!p)
            return;
        static_cast<RelayHardwareAdapter *>(p)->~RelayHardwareAdapter();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::relayAdapterDestructor() const
    {
        return &destroyRelayAdapter;
    }

    iotsmartsys::core::ICommandHardwareAdapter *ArduinoHardwareAdapterFactory::createRelay(
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
        if (!p)
            return;
        static_cast<OutputHardwareAdapter *>(p)->~OutputHardwareAdapter();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::outputAdapterDestructor() const
    {
        return &destroyOutputAdapter;
    }

    iotsmartsys::core::ICommandHardwareAdapter *ArduinoHardwareAdapterFactory::createOutput(
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
        if (!p)
            return;
        static_cast<InputHardwareAdapter *>(p)->~InputHardwareAdapter();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::inputAdapterDestructor() const
    {
        return &destroyInputAdapter;
    }

    iotsmartsys::core::IInputHardwareAdapter *ArduinoHardwareAdapterFactory::createInput(
        void *mem,
        std::uint8_t pin)
    {
        // ✅ sem heap, sem fragmentação
        return new (mem) InputHardwareAdapter(pin);
    }

    /* IWaterLevelSensor */
    std::size_t ArduinoHardwareAdapterFactory::waterLevelSensorAdapterSize() const
    {
        return sizeof(ArduinoUltrassonicWaterLevelSensor);
    }
    std::size_t ArduinoHardwareAdapterFactory::waterLevelSensorAdapterAlign() const
    {
        return alignof(ArduinoUltrassonicWaterLevelSensor);
    }
    static void destroyWaterLevelSensorAdapter(void *p)
    {
        if (!p)
            return;
        static_cast<ArduinoUltrassonicWaterLevelSensor *>(p)->~ArduinoUltrassonicWaterLevelSensor();
    }
    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::waterLevelSensorAdapterDestructor() const
    {
        return &destroyWaterLevelSensorAdapter;
    }
    iotsmartsys::core::IWaterLevelSensor *ArduinoHardwareAdapterFactory::createWaterLevelSensor(
        void *mem,
        std::uint8_t trigPin,
        std::uint8_t echoPin,
        float minLevelCm,
        float maxLevelCm,
        iotsmartsys::core::WaterLevelRecipentType recipentType)
    {
        auto *sr04Sensor = new SensorUltrassonicHCSR04(trigPin, echoPin, static_cast<long>(minLevelCm), static_cast<long>(maxLevelCm));

        return new (mem) ArduinoUltrassonicWaterLevelSensor(sr04Sensor, recipentType);
    }
} // namespace iotsmartsys::platform::arduino