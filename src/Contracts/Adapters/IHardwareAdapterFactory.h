#pragma once
#include <stdint.h>
#include <cstddef>
#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/WaterLevelRecipentType.h"
#include "Contracts/Sensors/IColorSensor.h"

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

        // Output Adapter
        virtual std::size_t outputAdapterSize() const = 0;
        virtual std::size_t outputAdapterAlign() const = 0;
        virtual ICommandHardwareAdapter *createOutput(void *mem, std::uint8_t pin, bool highIsOn) = 0;
        virtual AdapterDestructor outputAdapterDestructor() const = 0;

        // Input Adapter
        virtual std::size_t inputAdapterSize() const = 0;
        virtual std::size_t inputAdapterAlign() const = 0;
        virtual IInputHardwareAdapter *createInput(void *mem, std::uint8_t pin) = 0;
        virtual AdapterDestructor inputAdapterDestructor() const = 0;

        // IWaterLevelSensor
        virtual std::size_t waterLevelSensorAdapterSize() const = 0;
        virtual std::size_t waterLevelSensorAdapterAlign() const = 0;
        virtual IWaterLevelSensor *createWaterLevelSensor(void *mem, std::uint8_t trigPin, std::uint8_t echoPin, float minLevelCm, float maxLevelCm, core::WaterLevelRecipentType recipentType) = 0;
        virtual AdapterDestructor waterLevelSensorAdapterDestructor() const = 0;

        // IColorSensor
        // virtual std::size_t colorSensorAdapterSize() const = 0;
        // virtual std::size_t colorSensorAdapterAlign() const = 0;
        // virtual IColorSensor *createColorSensor(void *mem, std::uint8_t pin) = 0;
        // virtual AdapterDestructor colorSensorAdapterDestructor() const = 0;

    };

} // namespace iotsmartsys::core