#pragma once

#include <stdint.h>
#include <stddef.h>
#include <new>
#include <utility>

#include "App/Builders/Configs/HardwareConfig.h"
#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Capabilities/LightCapability.h"
#include "Contracts/Capabilities/AlarmCapability.h"
#include "Contracts/Capabilities/DoorSensorCapability.h"
#include "Contracts/Capabilities/PirSensorCapability.h"
#include "Contracts/Capabilities/SwitchPlugCapability.h"
#include "Contracts/Capabilities/ClapSensorCapability.h"
#include "Contracts/Capabilities/SwitchCapability.h"
#include "Contracts/Capabilities/PushButtonCapability.h"
#include "Contracts/Capabilities/TouchButtonCapability.h"
#include "Contracts/Capabilities/ValveCapability.h"
#include "Contracts/Capabilities/LEDCapability.h"
#include "Contracts/Capabilities/WaterFlowHallSensorCapability.h"
#include "Contracts/Capabilities/WaterLevelPercentCapability.h"
#include "Contracts/Capabilities/WaterLevelLitersCapability.h"
#include "Contracts/Capabilities/TemperatureSensorCapability.h"
#include "Contracts/Capabilities/HumiditySensorCapability.h"
#include "Contracts/Capabilities/HeightWaterLevelCapability.h"
#include "Contracts/Capabilities/GlpSensorCapability.h"
#include "Contracts/Capabilities/GlpMeterKgCapability.h"
#include "Contracts/Capabilities/GlpMeterPercentCapability.h"
#include "Contracts/Capabilities/OperationalColorSensorCapability.h"
#include "Contracts/Capabilities/Managers/CapabilityManager.h"
#include "Contracts/Capabilities/LuminosityCapability.h"
#include "Contracts/Providers/IDeviceIdentityProvider.h"

namespace iotsmartsys::app
{

    class CapabilitiesBuilder
    {
    public:
        using ICapability = iotsmartsys::core::ICapability;

        CapabilitiesBuilder(iotsmartsys::core::IHardwareAdapterFactory &factory,
                            iotsmartsys::core::ICapabilityEventSink &eventSink,
                            ICapability **capSlots,
                            void (**capDestructors)(void *),
                            size_t capSlotsMax,
                            void **adapterSlots,
                            void (**adapterDestructors)(void *),
                            size_t adapterSlotsMax,
                            uint8_t *arena,
                            size_t arenaBytes,
                            iotsmartsys::core::IDeviceIdentityProvider &deviceIdentityProvider);

        CapabilitiesBuilder(const CapabilitiesBuilder &) = delete;
        CapabilitiesBuilder &operator=(const CapabilitiesBuilder &) = delete;

        void reset();

        size_t count() const { return _count; }
        size_t remainingArenaBytes() const;

        iotsmartsys::core::CapabilityManager build() const;

        iotsmartsys::core::LightCapability *addLight(const LightConfig &cfg);
        iotsmartsys::core::AlarmCapability *addAlarm(const AlarmConfig &cfg);
        iotsmartsys::core::DoorSensorCapability *addDoorSensor(const DoorSensorConfig &cfg);
        iotsmartsys::core::PirSensorCapability *addPirSensor(const PirSensorConfig &cfg);
        iotsmartsys::core::SwitchPlugCapability *addSwitchPlug(const SwitchConfig &cfg);
        iotsmartsys::core::ClapSensorCapability *addClapSensor(const ClapSensorConfig &cfg);
        iotsmartsys::core::SwitchCapability *addSwitch(const SwitchConfig &cfg);
        iotsmartsys::core::PushButtonCapability *addPushButton(const PushButtonConfig &cfg);
        iotsmartsys::core::ValveCapability *addValve(const ValveConfig &cfg);
        iotsmartsys::core::LEDCapability *addLED(const LightConfig &cfg);
        iotsmartsys::core::WaterFlowHallSensorCapability *addWaterFlowHallSensor(const WaterFlowHallSensorConfig &cfg);
        iotsmartsys::core::WaterLevelPercentCapability *addWaterLevelPercent(const WaterLevelSensorConfig &cfg);
        iotsmartsys::core::WaterLevelLitersCapability *addWaterLevelLiters(const WaterLevelSensorConfig &cfg);
        iotsmartsys::core::TemperatureSensorCapability *addTemperatureSensor(const TemperatureSensorConfig &cfg);
        iotsmartsys::core::TouchButtonCapability *addTouchButton(const TouchButtonConfig &cfg);
        iotsmartsys::core::HumiditySensorCapability *addHumiditySensor(const HumiditySensorConfig &cfg);
        iotsmartsys::core::HeightWaterLevelCapability *addWaterHeight(const WaterLevelSensorConfig &cfg);
        iotsmartsys::core::GlpSensorCapability *addGlpSensor(const GlpSensorConfig &cfg);
        iotsmartsys::core::GlpMeterPercentCapability *addGlpMeterPercent(const GlpMeterConfig &cfg);
        iotsmartsys::core::GlpMeterKgCapability *addGlpMeterKg(const GlpMeterConfig &cfg);
        iotsmartsys::core::OperationalColorSensorCapability *addOperationalColorSensor(const OperationalColorSensorConfig &cfg);
        iotsmartsys::core::LuminosityCapability *addLuminosityCapability(const LuminositySensorConfig &cfg);

    private:
        iotsmartsys::core::IDeviceIdentityProvider &_deviceIdentityProvider;
        template <typename TCap, typename... Args>
        TCap *createCapability(Args &&...args)
        {
            if (_count >= _capsMax)
                return nullptr;

            void *memcap = allocateAligned(sizeof(TCap), alignof(TCap));
            if (!memcap)
                return nullptr;

            auto *cap = new (memcap) TCap(std::forward<Args>(args)...);

            auto dtor = [](void *p)
            {
                static_cast<TCap *>(p)->~TCap();
            };

            if (!registerCapability(cap, dtor))
            {
                cap->~TCap();
                return nullptr;
            }

            return cap;
        }

        iotsmartsys::core::ICommandHardwareAdapter *createOutputAdapter(std::uint8_t gpio, bool highIsOn);
        iotsmartsys::core::IInputHardwareAdapter *createInputAdapter(std::uint8_t gpio);

        void *allocateAligned(size_t sizeBytes, size_t alignment);
        bool registerCapability(ICapability *cap, void (*destructor)(void *));
        bool registerAdapter(void *adapter, void (*destructor)(void *));
    private:
        iotsmartsys::core::IHardwareAdapterFactory &_factory;
        iotsmartsys::core::ICapabilityEventSink &_eventSink;
        iotsmartsys::core::ICapability **_caps{nullptr};
        void (**_capDestructors)(void *){nullptr};
        size_t _capsMax{0};
        size_t _count{0};

        void **_adapters{nullptr};
        void (**_adapterDestructors)(void *){nullptr};
        size_t _adaptersMax{0};
        size_t _adaptersCount{0};

        uint8_t *_arena{nullptr};
        size_t _arenaBytes{0};
        size_t _arenaOffset{0};
    };

} // namespace iotsmartsys::app
