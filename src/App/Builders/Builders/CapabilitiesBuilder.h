#pragma once

#include <stdint.h>
#include <stddef.h>
#include <new>

#include "Contracts/Core/Capabilities/ICapability.h"
#include "Contracts/Core/Capabilities/LightCapability.h"
#include "App/Builders/Configs/LightConfig.h"
#include "Contracts/Core/Adapters/IHardwareAdapterFactory.h"

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
                                             void (**destructors)(void *),
                                             size_t capSlotsMax,
                                             uint8_t *arena,
                                             size_t arenaBytes);

        CapabilitiesBuilder(const CapabilitiesBuilder &) = delete;
        CapabilitiesBuilder &operator=(const CapabilitiesBuilder &) = delete;

        void reset();

        size_t count() const { return _count; }
        size_t remainingArenaBytes() const;

        CapabilityList build() const;

        iotsmartsys::core::LightCapability *addLight(const LightConfig &cfg);

    private:
        void *allocateAligned(size_t sizeBytes, size_t alignment);
        bool registerCapability(ICapability *cap, void (*destructor)(void *));

    private:
        iotsmartsys::core::IHardwareAdapterFactory& _factory;
        iotsmartsys::core::ICapability **_caps{nullptr};
        void (**_destructors)(void *){nullptr};
        size_t _capsMax{0};
        size_t _count{0};

        uint8_t *_arena{nullptr};
        size_t _arenaBytes{0};
        size_t _arenaOffset{0};
    };

} // namespace iotsmartsys::app