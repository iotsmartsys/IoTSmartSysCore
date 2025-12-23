#include "CapabilitiesBuilder.h"
#include "Contracts/Providers/ServiceProvider.h"

namespace iotsmartsys::app
{

    static inline size_t alignUp(size_t value, size_t alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    CapabilitiesBuilder::CapabilitiesBuilder(iotsmartsys::core::IHardwareAdapterFactory &factory,
                                             iotsmartsys::core::ICapabilityEventSink &eventSink,
                                             ICapability **capSlots,
                                             void (**capDestructors)(void *),
                                             size_t capSlotsMax,
                                             void **adapterSlots,
                                             void (**adapterDestructors)(void *),
                                             size_t adapterSlotsMax,
                                             uint8_t *arena,
                                             size_t arenaBytes)
        : _factory(factory),
          _eventSink(eventSink),
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

    iotsmartsys::core::CapabilityManager CapabilitiesBuilder::build() const
    {
        auto &settingsProvider = *iotsmartsys::core::ServiceProvider::instance().getSettingsProvider();
        auto &settingsGate = *iotsmartsys::core::ServiceProvider::instance().getSettingsGate();
        auto &logger = *iotsmartsys::core::ServiceProvider::instance().logger();
        logger.info("CAP_BUILDER", "Building CapabilityManager with %zu capabilities.", _count);
        iotsmartsys::core::CapabilityManager manager(_caps, _count,
                                                     settingsGate,
                                                     logger,
                                                     settingsProvider);

        logger.info("CAP_BUILDER", "CapabilityManager built successfully.");
        return manager;
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

        const std::size_t size = _factory.outputAdapterSize();
        const std::size_t align = _factory.outputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createOutput(
            mem,
            cfg.GPIO,
            cfg.highIsOn);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::LightCapability),
                                       alignof(iotsmartsys::core::LightCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::LightCapability(
            cfg.capability_name,
            *hardwareAdapter, &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::LightCapability *>(p)->~LightCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~LightCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addSwitch ---------------------------
    iotsmartsys::core::SwitchCapability *CapabilitiesBuilder::addSwitch(const SwitchPlugConfig &cfg)
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
            cfg.GPIO,
            cfg.highIsOn);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::SwitchCapability),
                                       alignof(iotsmartsys::core::SwitchCapability));
        if (!memcap)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();

        auto *cap = new (memcap) iotsmartsys::core::SwitchCapability(
            name,
            *hardwareAdapter, &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::SwitchCapability *>(p)->~SwitchCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~SwitchCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addPushButton ---------------------------
    iotsmartsys::core::PushButtonCapability *CapabilitiesBuilder::addPushButton(const PushButtonConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(
            mem,
            cfg.GPIO);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::PushButtonCapability),
                                       alignof(iotsmartsys::core::PushButtonCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::PushButtonCapability(
            *static_cast<iotsmartsys::core::IInputHardwareAdapter *>(hardwareAdapter),
            &_eventSink,
            static_cast<unsigned long>(cfg.debounceTimeMs));

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::PushButtonCapability *>(p)->~PushButtonCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~PushButtonCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addTouchButton ---------------------------
    iotsmartsys::core::TouchButtonCapability *CapabilitiesBuilder::addTouchButton(const PushButtonConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(
            mem,
            cfg.GPIO);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::TouchButtonCapability),
                                       alignof(iotsmartsys::core::TouchButtonCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::TouchButtonCapability(
            *static_cast<iotsmartsys::core::IInputHardwareAdapter *>(hardwareAdapter),
            &_eventSink,
            static_cast<unsigned long>(cfg.debounceTimeMs));

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::TouchButtonCapability *>(p)->~TouchButtonCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~TouchButtonCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addValve ---------------------------
    iotsmartsys::core::ValveCapability *CapabilitiesBuilder::addValve(const SwitchPlugConfig &cfg)
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
            cfg.GPIO,
            cfg.highIsOn);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::ValveCapability),
                                       alignof(iotsmartsys::core::ValveCapability));
        if (!memcap)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();

        auto *cap = new (memcap) iotsmartsys::core::ValveCapability(
            name,
            *hardwareAdapter,
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::ValveCapability *>(p)->~ValveCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~ValveCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addLED ---------------------------
    iotsmartsys::core::LEDCapability *CapabilitiesBuilder::addLED(const LightConfig &cfg)
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
            cfg.GPIO,
            cfg.highIsOn);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::LEDCapability),
                                       alignof(iotsmartsys::core::LEDCapability));
        if (!memcap)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();

        auto *cap = new (memcap) iotsmartsys::core::LEDCapability(
            name,
            *hardwareAdapter,
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::LEDCapability *>(p)->~LEDCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~LEDCapability();
            return nullptr;
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
            cfg.GPIO,
            cfg.highIsOn);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::AlarmCapability),
                                       alignof(iotsmartsys::core::AlarmCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::AlarmCapability(*hardwareAdapter, &_eventSink);

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

    // --------------------------- addDoorSensor ---------------------------
    iotsmartsys::core::DoorSensorCapability *CapabilitiesBuilder::addDoorSensor(const DoorSensorConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(
            mem,
            cfg.GPIO);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::DoorSensorCapability),
                                       alignof(iotsmartsys::core::DoorSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::DoorSensorCapability(
            *static_cast<iotsmartsys::core::IInputHardwareAdapter *>(hardwareAdapter),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::DoorSensorCapability *>(p)->~DoorSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~DoorSensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addPirSensor ---------------------------
    iotsmartsys::core::PirSensorCapability *CapabilitiesBuilder::addPirSensor(const PirSensorConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(
            mem,
            cfg.GPIO);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::PirSensorCapability),
                                       alignof(iotsmartsys::core::PirSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::PirSensorCapability(
            *static_cast<iotsmartsys::core::IInputHardwareAdapter *>(hardwareAdapter),
            &_eventSink,
            cfg.debounceTimeMs);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::PirSensorCapability *>(p)->~PirSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~PirSensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addClapSensor ---------------------------
    iotsmartsys::core::ClapSensorCapability *CapabilitiesBuilder::addClapSensor(const ClapSensorConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(
            mem,
            cfg.GPIO);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::ClapSensorCapability),
                                       alignof(iotsmartsys::core::ClapSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::ClapSensorCapability(
            *static_cast<iotsmartsys::core::IInputHardwareAdapter *>(hardwareAdapter),
            &_eventSink,
            cfg.debounceTimeMs);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::ClapSensorCapability *>(p)->~ClapSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~ClapSensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addSwitchPlug ---------------------------
    iotsmartsys::core::SwitchPlugCapability *CapabilitiesBuilder::addSwitchPlug(const SwitchPlugConfig &cfg)
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
            cfg.GPIO,
            cfg.highIsOn);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::SwitchPlugCapability),
                                       alignof(iotsmartsys::core::SwitchPlugCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::SwitchPlugCapability(
            std::string(),
            *hardwareAdapter,
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::SwitchPlugCapability *>(p)->~SwitchPlugCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~SwitchPlugCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addWaterFlowHallSensor ---------------------------
    iotsmartsys::core::WaterFlowHallSensorCapability *CapabilitiesBuilder::addWaterFlowHallSensor(const WaterFlowHallSensorConfig &cfg)
    {
        if (_count >= _capsMax || _adaptersCount >= _adaptersMax)
            return nullptr;
        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();
        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(
            mem,
            cfg.GPIO);

        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::WaterFlowHallSensorCapability),
                                       alignof(iotsmartsys::core::WaterFlowHallSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::WaterFlowHallSensorCapability(
            *static_cast<iotsmartsys::core::IInputHardwareAdapter *>(hardwareAdapter),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::WaterFlowHallSensorCapability *>(p)->~WaterFlowHallSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~WaterFlowHallSensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addWaterLevelPercent ---------------------------
    iotsmartsys::core::WaterLevelPercentCapability *CapabilitiesBuilder::addWaterLevelPercent(const WaterLevelSensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::WaterLevelPercentCapability),
                                       alignof(iotsmartsys::core::WaterLevelPercentCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::WaterLevelPercentCapability(
            *static_cast<iotsmartsys::core::IWaterLevelSensor *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::WaterLevelPercentCapability *>(p)->~WaterLevelPercentCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~WaterLevelPercentCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addWaterLevelLiters ---------------------------
    iotsmartsys::core::WaterLevelLitersCapability *CapabilitiesBuilder::addWaterLevelLiters(const WaterLevelSensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::WaterLevelLitersCapability),
                                       alignof(iotsmartsys::core::WaterLevelLitersCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::WaterLevelLitersCapability(
            *static_cast<iotsmartsys::core::IWaterLevelSensor *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::WaterLevelLitersCapability *>(p)->~WaterLevelLitersCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~WaterLevelLitersCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addTemperatureSensor ---------------------------
    iotsmartsys::core::TemperatureSensorCapability *CapabilitiesBuilder::addTemperatureSensor(const TemperatureSensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::TemperatureSensorCapability),
                                       alignof(iotsmartsys::core::TemperatureSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::TemperatureSensorCapability(
            *static_cast<iotsmartsys::core::ITemperatureSensor *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::TemperatureSensorCapability *>(p)->~TemperatureSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~TemperatureSensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addHumiditySensor ---------------------------
    iotsmartsys::core::HumiditySensorCapability *CapabilitiesBuilder::addHumiditySensor(const HumiditySensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::HumiditySensorCapability),
                                       alignof(iotsmartsys::core::HumiditySensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::HumiditySensorCapability(
            *static_cast<iotsmartsys::core::IHumiditySensor *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::HumiditySensorCapability *>(p)->~HumiditySensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~HumiditySensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addWaterHeight ---------------------------
    iotsmartsys::core::HeightWaterLevelCapability *CapabilitiesBuilder::addWaterHeight(const WaterLevelSensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::HeightWaterLevelCapability),
                                       alignof(iotsmartsys::core::HeightWaterLevelCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::HeightWaterLevelCapability(
            *static_cast<iotsmartsys::core::IWaterLevelSensor *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::HeightWaterLevelCapability *>(p)->~HeightWaterLevelCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~HeightWaterLevelCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addGlpSensor ---------------------------
    iotsmartsys::core::GlpSensorCapability *CapabilitiesBuilder::addGlpSensor(const GlpSensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::GlpSensorCapability),
                                       alignof(iotsmartsys::core::GlpSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::GlpSensorCapability(
            *static_cast<iotsmartsys::core::IGlpSensor *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::GlpSensorCapability *>(p)->~GlpSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~GlpSensorCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addGlpMeter ---------------------------
    iotsmartsys::core::GlpMeterCapability *CapabilitiesBuilder::addGlpMeter(const GlpMeterConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::GlpMeterCapability),
                                       alignof(iotsmartsys::core::GlpMeterCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::GlpMeterCapability(
            *static_cast<iotsmartsys::core::IGlpMeter *>(cfg.sensor),
            &_eventSink);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::GlpMeterCapability *>(p)->~GlpMeterCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~GlpMeterCapability();
            return nullptr;
        }

        return cap;
    }

    // --------------------------- addOperationalColorSensor ---------------------------
    iotsmartsys::core::OperationalColorSensorCapability *CapabilitiesBuilder::addOperationalColorSensor(const OperationalColorSensorConfig &cfg)
    {
        if (_count >= _capsMax)
            return nullptr;

        void *memcap = allocateAligned(sizeof(iotsmartsys::core::OperationalColorSensorCapability),
                                       alignof(iotsmartsys::core::OperationalColorSensorCapability));
        if (!memcap)
            return nullptr;

        auto *cap = new (memcap) iotsmartsys::core::OperationalColorSensorCapability(
            *static_cast<iotsmartsys::core::IColorSensor *>(cfg.sensor), &_eventSink, cfg.debounceTimeMs);

        auto dtor = [](void *p)
        {
            static_cast<iotsmartsys::core::OperationalColorSensorCapability *>(p)->~OperationalColorSensorCapability();
        };

        if (!registerCapability(cap, dtor))
        {
            cap->~OperationalColorSensorCapability();
            return nullptr;
        }

        return cap;
    }

} // namespace iotsmartsys::app