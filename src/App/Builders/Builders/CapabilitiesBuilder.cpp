#include "CapabilitiesBuilder.h"
#include "Contracts/Core/Adapters/IHardwareAdapterFactory.h"

namespace iotsmartsys::app
{

    static inline size_t alignUp(size_t value, size_t alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    CapabilitiesBuilder::CapabilitiesBuilder(iotsmartsys::core::IHardwareAdapterFactory& factory,
                                             ICapability **capSlots,
                                             void (**destructors)(void *),
                                             size_t capSlotsMax,
                                             uint8_t *arena,
                                             size_t arenaBytes)
        : _factory(factory),
          _caps(capSlots),
          _destructors(destructors),
          _capsMax(capSlotsMax),
          _arena(arena),
          _arenaBytes(arenaBytes)
    {
        // Zera contadores
        _count = 0;
        _arenaOffset = 0;

        // (Opcional) zerar arrays se quiser
        // for (size_t i = 0; i < _capsMax; ++i) { _caps[i] = nullptr; _destructors[i] = nullptr; }
    }

    void CapabilitiesBuilder::reset()
    {
        for (size_t i = _count; i > 0; --i)
        {
            const size_t idx = i - 1;
            if (_caps[idx] && _destructors[idx])
            {
                _destructors[idx]((void *)_caps[idx]);
            }
            _caps[idx] = nullptr;
            _destructors[idx] = nullptr;
        }

        _count = 0;
        _arenaOffset = 0;
    }

    size_t CapabilitiesBuilder::remainingArenaBytes() const
    {
        if (_arenaOffset >= _arenaBytes)
            return 0;
        return _arenaBytes - _arenaOffset;
    }

    CapabilityList CapabilitiesBuilder::build() const
    {
        CapabilityList list;
        list.items = _caps;
        list.count = _count;
        return list;
    }

    void *CapabilitiesBuilder::allocateAligned(size_t sizeBytes, size_t alignment)
    {
        const size_t aligned = alignUp(_arenaOffset, alignment);
        const size_t needed = aligned + sizeBytes;

        if (needed > _arenaBytes)
            return nullptr;

        void *mem = (void *)(&_arena[aligned]);
        _arenaOffset = needed;
        return mem;
    }

    bool CapabilitiesBuilder::registerCapability(ICapability *cap, void (*destructor)(void *))
    {
        if (_count >= _capsMax)
            return false;

        _caps[_count] = cap;
        _destructors[_count] = destructor;
        _count++;
        return true;
    }

    // --------------------------- addLight ---------------------------

    iotsmartsys::core::LightCapability *CapabilitiesBuilder::addLight(const LightConfig &cfg)
    {
        const std::size_t size = _factory.relayAdapterSize();
        const std::size_t align = _factory.relayAdapterAlign();

        void *mem = allocateAligned(size, align); // sua arena
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createRelay(
            mem,
            cfg.pin,
            cfg.activeHigh);

        if (!hardwareAdapter)
            return nullptr;

        // 1) Reserva memória alinhada para LightCapability
        void *memcap = allocateAligned(sizeof(iotsmartsys::core::LightCapability),
                                       alignof(iotsmartsys::core::LightCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::LightCapability(
            cfg.capability_name,
            *hardwareAdapter);

        // 3) Registra destructor (sem precisar de virtual destructor)
        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::LightCapability *>(p)->~LightCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            // Sem slot: chama destrutor manualmente (arena fica “perdida”, mas é ok; você pode resetar tudo depois)
            cap->~LightCapability();
            return nullptr;
        }

        // 4) Estado inicial (se fizer sentido no teu design)
        if (cfg.initialOn)
        {
            // Se sua LightCapability tiver método de ligar, use aqui.
            // Se não tiver, você pode só refletir state:
            cap->updateState("on");
        }
        else
        {
            cap->updateState("off");
        }

        return cap;
    }

} // namespace iotsmartsys::app