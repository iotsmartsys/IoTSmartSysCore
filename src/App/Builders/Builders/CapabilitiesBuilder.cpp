#include "CapabilitiesBuilder.h"

namespace iotsmartsys::app
{

    static inline size_t alignUp(size_t value, size_t alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    CapabilitiesBuilder::CapabilitiesBuilder(iotsmartsys::core::IHardwareAdapterFactory &factory,
                                             ICapability **capSlots,
                                             void (**capDestructors)(void *),
                                             size_t capSlotsMax,
                                             void **adapterSlots,
                                             void (**adapterDestructors)(void *),
                                             size_t adapterSlotsMax,
                                             uint8_t *arena,
                                             size_t arenaBytes)
        : _factory(factory),
          _caps(capSlots),
          _capDestructors(capDestructors),
          _capsMax(capSlotsMax),
          _adapters(adapterSlots),
          _adapterDestructors(adapterDestructors),
          _adaptersMax(adapterSlotsMax),
          _arena(arena),
          _arenaBytes(arenaBytes)
    {
        _count = 0;
        _adaptersCount = 0;
        _arenaOffset = 0;
    }

    void CapabilitiesBuilder::reset()
    {
        for (size_t i = _count; i > 0; --i)
        {
            const size_t idx = i - 1;
            if (_caps[idx] && _capDestructors[idx])
            {
                _capDestructors[idx]((void *)_caps[idx]);
            }
            _caps[idx] = nullptr;
            _capDestructors[idx] = nullptr;
        }

        for (size_t i = _adaptersCount; i > 0; --i)
        {
            const size_t idx = i - 1;
            if (_adapters[idx] && _adapterDestructors[idx])
            {
                _adapterDestructors[idx](_adapters[idx]);
            }
            _adapters[idx] = nullptr;
            _adapterDestructors[idx] = nullptr;
        }

        _count = 0;
        _adaptersCount = 0;
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
        _capDestructors[_count] = destructor;
        _count++;
        return true;
    }

    bool CapabilitiesBuilder::registerAdapter(void *adapter, void (*destructor)(void *))
    {
        if (_adaptersCount >= _adaptersMax)
            return false;

        _adapters[_adaptersCount] = adapter;
        _adapterDestructors[_adaptersCount] = destructor;
        _adaptersCount++;
        return true;
    }

    // --------------------------- addLight ---------------------------

    iotsmartsys::core::LightCapability *CapabilitiesBuilder::addLight(const LightConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.relayAdapterSize();
        const std::size_t align = _factory.relayAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createRelay(
            mem,
            cfg.pin,
            cfg.activeHigh);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.relayAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::LightCapability),
                                       alignof(iotsmartsys::core::LightCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::LightCapability(
            cfg.capability_name,
            *hardwareAdapter);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::LightCapability *>(p)->~LightCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~LightCapability();
            return nullptr;
        }

        if (cfg.initialOn)
        {
            cap->updateState("on");
        }
        else
        {
            cap->updateState("off");
        }

        return cap;
    }

    // --------------------------- addAlarm ---------------------------

    iotsmartsys::core::AlarmCapability *CapabilitiesBuilder::addAlarm(const AlarmConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.outputAdapterSize();
        const std::size_t align = _factory.outputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createOutput(
            mem,
            cfg.pin,
            cfg.activeState);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::AlarmCapability),
                                       alignof(iotsmartsys::core::AlarmCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::AlarmCapability(*hardwareAdapter);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::AlarmCapability *>(p)->~AlarmCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~AlarmCapability();
            return nullptr;
        }

        return cap;
    }

} // namespace iotsmartsys::app