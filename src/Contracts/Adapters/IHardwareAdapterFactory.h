#pragma once
#include <stdint.h>
#include <cstddef>
#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Adapters/IInputHardwareAdapter.h"
#include "Contracts/Sensors/IWaterLevelSensor.h"
#include "Contracts/Sensors/WaterLevelRecipentType.h"
#include "Contracts/Sensors/IColorSensor.h"
#include "Contracts/Sensors/ICurrentSensor.h"
#include "Contracts/Sensors/IVoltageSensor.h"

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
        virtual IInputHardwareAdapter *createInput(void *mem, std::uint8_t pin, HardwareDigitalLogic mode, InputPullMode pullMode) = 0;
        virtual AdapterDestructor inputAdapterDestructor() const = 0;

        // IWaterLevelSensor
        virtual std::size_t waterLevelSensorAdapterSize() const = 0;
        virtual std::size_t waterLevelSensorAdapterAlign() const = 0;
        virtual IWaterLevelSensor *createWaterLevelSensor(void *mem, std::uint8_t trigPin, std::uint8_t echoPin, float minLevelCm, float maxLevelCm, core::WaterLevelRecipentType recipentType) = 0;
        virtual AdapterDestructor waterLevelSensorAdapterDestructor() const = 0;

        // Current sensor extension. Defaults preserve existing factory
        // implementations that do not support the contracted ESP32 target.
        virtual bool currentSensorTargetSupported() const { return false; }
        virtual bool currentSensorPinHasAdc(int) const { return false; }
        virtual bool currentSensorPinReserved(int) const { return true; }
        virtual std::size_t currentSensorAdapterSize() const { return 0; }
        virtual std::size_t currentSensorAdapterAlign() const { return 0; }
        virtual ICurrentSensor *createCurrentSensor(void *, const CurrentSensorConfig &) { return nullptr; }
        virtual AdapterDestructor currentSensorAdapterDestructor() const { return nullptr; }

        // Voltage sensor extension. Defaults preserve factories without this
        // adapter and keep target-specific ADC validation in the platform.
        virtual bool voltageSensorTargetSupported() const { return false; }
        virtual bool voltageSensorPinHasAdc(int) const { return false; }
        virtual bool voltageSensorPinReserved(int) const { return true; }
        virtual std::size_t voltageSensorAdapterSize() const { return 0; }
        virtual std::size_t voltageSensorAdapterAlign() const { return 0; }
        virtual IVoltageSensor *createVoltageSensor(void *, const VoltageSensorConfig &) { return nullptr; }
        virtual AdapterDestructor voltageSensorAdapterDestructor() const { return nullptr; }

        // IColorSensor
        // virtual std::size_t colorSensorAdapterSize() const = 0;
        // virtual std::size_t colorSensorAdapterAlign() const = 0;
        // virtual IColorSensor *createColorSensor(void *mem, std::uint8_t pin) = 0;
        // virtual AdapterDestructor colorSensorAdapterDestructor() const = 0;

    };

} // namespace iotsmartsys::core
