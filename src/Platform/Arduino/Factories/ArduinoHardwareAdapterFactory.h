#pragma once

#include <cstddef>
#include <cstdint>

#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"

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
        iotsmartsys::core::ICommandHardwareAdapter *createRelay(void *mem, std::uint8_t pin, bool highIsOn) override;
        AdapterDestructor relayAdapterDestructor() const override;

        /* Output Adapter */
        std::size_t outputAdapterSize() const override;
        std::size_t outputAdapterAlign() const override;
        iotsmartsys::core::ICommandHardwareAdapter *createOutput(void *mem, std::uint8_t pin, bool highIsOn) override;
        AdapterDestructor outputAdapterDestructor() const override;

        /* Input Adapter */
        std::size_t inputAdapterSize() const override;
        std::size_t inputAdapterAlign() const override;
        iotsmartsys::core::IInputHardwareAdapter *createInput(void *mem, std::uint8_t pin) override;
        AdapterDestructor inputAdapterDestructor() const override;

        /* IWaterLevelSensor */
        std::size_t waterLevelSensorAdapterSize() const override;
        std::size_t waterLevelSensorAdapterAlign() const override;
        iotsmartsys::core::IWaterLevelSensor *createWaterLevelSensor(void *mem, std::uint8_t trigPin, std::uint8_t echoPin, float minLevelCm, float maxLevelCm, iotsmartsys::core::WaterLevelRecipentType recipentType) override;
        AdapterDestructor waterLevelSensorAdapterDestructor() const override;

    };

} // namespace iotsmartsys::platform::arduino