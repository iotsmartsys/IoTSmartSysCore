#include <Arduino.h>

#include "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.h"
#include "Platform/Arduino/Adapters/OutputHardwareAdapter.h"
#include "Platform/Arduino/Adapters/InputHardwareAdapter.h"
#include "Platform/Arduino/Sensors/ACS712C30ACurrentSensor.h"
#include "Config/BuildConfig.h"
#if IOTSMARTSYS_SENSORS_ENABLED
#include "Platform/Arduino/Sensors/ArduinoUltrassonicWaterLevelSensor.h"
#endif

namespace iotsmartsys::platform::arduino
{
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
                   ? iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON
                   : iotsmartsys::core::HardwareDigitalLogic::LOW_IS_ON;

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

    iotsmartsys::core::IInputHardwareAdapter *ArduinoHardwareAdapterFactory::createInput(
        void *mem,
        std::uint8_t pin,
        HardwareDigitalLogic mode,
        InputPullMode pullMode)
    {
        // ✅ sem heap, sem fragmentação
        return new (mem) InputHardwareAdapter(pin, mode, pullMode);
    }

    /* IWaterLevelSensor */
    std::size_t ArduinoHardwareAdapterFactory::waterLevelSensorAdapterSize() const
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        return sizeof(ArduinoUltrassonicWaterLevelSensor);
#else
        return 0;
#endif
    }
    std::size_t ArduinoHardwareAdapterFactory::waterLevelSensorAdapterAlign() const
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        return alignof(ArduinoUltrassonicWaterLevelSensor);
#else
        return 0;
#endif
    }
    static void destroyWaterLevelSensorAdapter(void *p)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        if (!p)
            return;
        static_cast<ArduinoUltrassonicWaterLevelSensor *>(p)->~ArduinoUltrassonicWaterLevelSensor();
#else
        (void)p;
#endif
    }
    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::waterLevelSensorAdapterDestructor() const
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        return &destroyWaterLevelSensorAdapter;
#else
        return nullptr;
#endif
    }
    iotsmartsys::core::IWaterLevelSensor *ArduinoHardwareAdapterFactory::createWaterLevelSensor(
        void *mem,
        std::uint8_t trigPin,
        std::uint8_t echoPin,
        float minLevelCm,
        float maxLevelCm,
        iotsmartsys::core::WaterLevelRecipentType recipentType)
    {
#if IOTSMARTSYS_SENSORS_ENABLED
        SensorUltrassonicHCSR04Config ultrasonicCfg;
        ultrasonicCfg.triggerPin = trigPin;
        ultrasonicCfg.echoPin = echoPin;
        ultrasonicCfg.minDistance = static_cast<long>(minLevelCm);
        ultrasonicCfg.maxDistance = static_cast<long>(maxLevelCm);
        auto *sr04Sensor = new SensorUltrassonicHCSR04(ultrasonicCfg);

        return new (mem) ArduinoUltrassonicWaterLevelSensor(sr04Sensor, recipentType);
#else
        (void)mem;
        (void)trigPin;
        (void)echoPin;
        (void)minLevelCm;
        (void)maxLevelCm;
        (void)recipentType;
        return nullptr;
#endif
    }

    bool ArduinoHardwareAdapterFactory::currentSensorTargetSupported() const
    {
#if defined(CONFIG_IDF_TARGET_ESP32)
        return true;
#else
        return false;
#endif
    }

    bool ArduinoHardwareAdapterFactory::currentSensorPinHasAdc(int pin) const
    {
#if defined(CONFIG_IDF_TARGET_ESP32)
        return pin >= 0 && pin < NUM_DIGITAL_PINS &&
               digitalPinToAnalogChannel(static_cast<std::uint8_t>(pin)) >= 0;
#else
        (void)pin;
        return false;
#endif
    }

    bool ArduinoHardwareAdapterFactory::currentSensorPinReserved(int pin) const
    {
#if defined(CONFIG_IDF_TARGET_ESP32)
        // The supported runtime keeps Wi-Fi active. On classic ESP32 the ADC2
        // controller is shared with Wi-Fi, so current sensing is constrained
        // to the ADC1 GPIOs 32..39.
        return currentSensorPinHasAdc(pin) && (pin < 32 || pin > 39);
#else
        (void)pin;
        return true;
#endif
    }

    std::size_t ArduinoHardwareAdapterFactory::currentSensorAdapterSize() const
    {
        return currentSensorTargetSupported() ? sizeof(ACS712C30ACurrentSensor) : 0;
    }

    std::size_t ArduinoHardwareAdapterFactory::currentSensorAdapterAlign() const
    {
        return currentSensorTargetSupported() ? alignof(ACS712C30ACurrentSensor) : 0;
    }

    static void destroyCurrentSensorAdapter(void *p)
    {
        if (!p)
            return;
        static_cast<ACS712C30ACurrentSensor *>(p)->~ACS712C30ACurrentSensor();
    }

    ArduinoHardwareAdapterFactory::AdapterDestructor ArduinoHardwareAdapterFactory::currentSensorAdapterDestructor() const
    {
        return currentSensorTargetSupported() ? &destroyCurrentSensorAdapter : nullptr;
    }

    iotsmartsys::core::ICurrentSensor *ArduinoHardwareAdapterFactory::createCurrentSensor(
        void *mem,
        const iotsmartsys::core::CurrentSensorConfig &config)
    {
        if (!mem || !currentSensorTargetSupported())
            return nullptr;
        return new (mem) ACS712C30ACurrentSensor(config);
    }

    /* IColorSensor */
    // std::size_t ArduinoHardwareAdapterFactory::colorSensorAdapterSize() const
    // {
    //    return size
    // }
} // namespace iotsmartsys::platform::arduino
