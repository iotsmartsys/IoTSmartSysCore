#pragma once

#include <stdint.h>
#include <stddef.h>
#include <new>

#include "App/Builders/Configs/LightConfig.h"
#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Capabilities/LightCapability.h"
#include "Contracts/Capabilities/AlarmCapability.h"
#include "Contracts/Capabilities/DoorSensorCapability.h"

namespace iotsmartsys::app
{

    struct CapabilityList
    {
        iotsmartsys::core::ICapability *const *items{nullptr};
        size_t count{0};

        iotsmartsys::core::ICapability *operator[](size_t i) const { return items[i]; }
    };

    class CapabilitiesBuilder
    {
    public:
        using ICapability = iotsmartsys::core::ICapability;

        CapabilitiesBuilder(iotsmartsys::core::IHardwareAdapterFactory& factory,
                            ICapability **capSlots,
                            void (**capDestructors)(void *),
                            size_t capSlotsMax,
                            void **adapterSlots,
                            void (**adapterDestructors)(void *),
                            size_t adapterSlotsMax,
                            uint8_t *arena,
                            size_t arenaBytes);

        CapabilitiesBuilder(const CapabilitiesBuilder &) = delete;
        CapabilitiesBuilder &operator=(const CapabilitiesBuilder &) = delete;

        void reset();

        size_t count() const { return _count; }
        size_t remainingArenaBytes() const;

        CapabilityList build() const;

        iotsmartsys::core::LightCapability *addLight(const LightConfig &cfg);
        iotsmartsys::core::AlarmCapability *addAlarm(const AlarmConfig &cfg);
        iotsmartsys::core::DoorSensorCapability *addDoorSensor(const DoorSensorConfig &cfg);

    private:
        void *allocateAligned(size_t sizeBytes, size_t alignment);
        bool registerCapability(ICapability *cap, void (*destructor)(void *));
        bool registerAdapter(void *adapter, void (*destructor)(void *));

    private:
        iotsmartsys::core::IHardwareAdapterFactory& _factory;
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