#include "CapabilitiesBuilder.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.h"

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
                                             size_t arenaBytes,
                                             iotsmartsys::core::IDeviceIdentityProvider &deviceIdentityProvider)
        : _factory(factory),
          _eventSink(eventSink),
          _caps(capSlots),
          _capDestructors(capDestructors),
          _capsMax(capSlotsMax),
          _adapters(adapterSlots),
          _adapterDestructors(adapterDestructors),
          _adaptersMax(adapterSlotsMax),
          _arena(arena),
          _arenaBytes(arenaBytes),
          _deviceIdentityProvider(deviceIdentityProvider)
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
        {
            return false;
        }

        if (cap->capability_name.empty())
        {
            auto deviceId = _deviceIdentityProvider.getDeviceID();
            cap->capability_name = deviceId + "_" + cap->type;
        }

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

    iotsmartsys::core::ICommandHardwareAdapter *CapabilitiesBuilder::createOutputAdapter(std::uint8_t gpio, bool highIsOn)
    {
        if (_adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.outputAdapterSize();
        const std::size_t align = _factory.outputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createOutput(mem, gpio, highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.outputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        return hardwareAdapter;
    }

    iotsmartsys::core::IInputHardwareAdapter *CapabilitiesBuilder::createInputAdapter(std::uint8_t gpio)
    {
        if (_adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(mem, gpio);
        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        return hardwareAdapter;
    }

    iotsmartsys::core::IInputHardwareAdapter *CapabilitiesBuilder::createInputAdapter(std::uint8_t gpio, iotsmartsys::core::HardwareDigitalLogic mode, iotsmartsys::core::InputPullMode pullMode)
    {
        if (_adaptersCount >= _adaptersMax)
            return nullptr;

        const std::size_t size = _factory.inputAdapterSize();
        const std::size_t align = _factory.inputAdapterAlign();

        void *mem = allocateAligned(size, align);
        if (!mem)
            return nullptr;

        auto *hardwareAdapter = _factory.createInput(mem, gpio, mode, pullMode);
        if (!hardwareAdapter)
            return nullptr;

        auto adapterDtor = _factory.inputAdapterDestructor();
        if (!registerAdapter(hardwareAdapter, adapterDtor))
            return nullptr;

        return hardwareAdapter;
    }

    // --------------------------- addLight ---------------------------

    iotsmartsys::core::LightCapability *CapabilitiesBuilder::addLight(const LightConfig &cfg)
    {
        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::LightCapability>(
            name,
            *hardwareAdapter, &_eventSink);
    }

    // --------------------------- addSwitch ---------------------------
    iotsmartsys::core::SwitchCapability *CapabilitiesBuilder::addSwitch(const SwitchConfig &cfg)
    {
        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::SwitchCapability>(
            name,
            *hardwareAdapter, &_eventSink);
    }

    // --------------------------- addPushButton ---------------------------
    iotsmartsys::core::PushButtonCapability *CapabilitiesBuilder::addPushButton(const PushButtonConfig &cfg)
    {
        auto *hardwareAdapter = createInputAdapter(cfg.GPIO);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::PushButtonCapability>(
            name,
            *hardwareAdapter,
            &_eventSink,
            static_cast<unsigned long>(cfg.debounceTimeMs));
    }

    // --------------------------- addTouchButton ---------------------------
    iotsmartsys::core::TouchButtonCapability *CapabilitiesBuilder::addTouchButton(const TouchButtonConfig &cfg)
    {
        auto *hardwareAdapter = createInputAdapter(cfg.GPIO);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::TouchButtonCapability>(
            name,
            *hardwareAdapter,
            &_eventSink,
            static_cast<unsigned long>(cfg.debounceTimeMs));
    }

    // --------------------------- addValve ---------------------------
    iotsmartsys::core::ValveCapability *CapabilitiesBuilder::addValve(const ValveConfig &cfg)
    {
        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        auto *cap = createCapability<iotsmartsys::core::ValveCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);

        if (!cap)
            return nullptr;

        void *interpreterMem = allocateAligned(
            sizeof(iotsmartsys::core::ValveHardwareCommandInterpreter),
            alignof(iotsmartsys::core::ValveHardwareCommandInterpreter));

        if (!interpreterMem)
            return cap;

        auto *interpreter = new (interpreterMem) iotsmartsys::core::ValveHardwareCommandInterpreter();
        cap->setCommandInterpreter(interpreter);

        return cap;
    }

    // --------------------------- addLED ---------------------------
    iotsmartsys::core::LEDCapability *CapabilitiesBuilder::addLED(const LightConfig &cfg)
    {
        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::LEDCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addAlarm ---------------------------

    iotsmartsys::core::AlarmCapability *CapabilitiesBuilder::addAlarm(const AlarmConfig &cfg)
    {
        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::AlarmCapability>(*hardwareAdapter, &_eventSink);
    }

    // --------------------------- addDoorSensor ---------------------------
    iotsmartsys::core::DoorSensorCapability *CapabilitiesBuilder::addDoorSensor(const DoorSensorConfig &cfg)
    {
        auto *hardwareAdapter = createInputAdapter(cfg.GPIO, iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON, iotsmartsys::core::InputPullMode::PULL_UP);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::DoorSensorCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addPirSensor ---------------------------
    iotsmartsys::core::PirSensorCapability *CapabilitiesBuilder::addPirSensor(const PirSensorConfig &cfg)
    {
        auto *hardwareAdapter = createInputAdapter(cfg.GPIO, cfg.highIsOn ? iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON : iotsmartsys::core::HardwareDigitalLogic::LOW_IS_ON, iotsmartsys::core::InputPullMode::NONE);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::PirSensorCapability>(
            name,
            *hardwareAdapter,
            &_eventSink,
            cfg.debounceTimeMs);
    }

    // --------------------------- addClapSensor ---------------------------
    iotsmartsys::core::ClapSensorCapability *CapabilitiesBuilder::addClapSensor(const ClapSensorConfig &cfg)
    {
        const auto logic = cfg.highIsOn ? iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON
                                        : iotsmartsys::core::HardwareDigitalLogic::LOW_IS_ON;
        const auto pullMode = cfg.highIsOn ? iotsmartsys::core::InputPullMode::PULL_DOWN
                                           : iotsmartsys::core::InputPullMode::PULL_UP;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO, logic, pullMode);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::ClapSensorCapability>(
            name.c_str(),
            *hardwareAdapter,
            &_eventSink,
            cfg.debounceTimeMs);
    }

    // --------------------------- addSwitchPlug ---------------------------
    iotsmartsys::core::SwitchPlugCapability *CapabilitiesBuilder::addSwitchPlug(const SwitchConfig &cfg)
    {
        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::SwitchPlugCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addWaterFlowHallSensor ---------------------------
    iotsmartsys::core::WaterFlowHallSensorCapability *CapabilitiesBuilder::addWaterFlowHallSensor(const WaterFlowHallSensorConfig &cfg)
    {
        auto *hardwareAdapter = createInputAdapter(cfg.GPIO);
        if (!hardwareAdapter)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::WaterFlowHallSensorCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addWaterLevelPercent ---------------------------
    iotsmartsys::core::WaterLevelPercentCapability *CapabilitiesBuilder::addWaterLevelPercent(const WaterLevelSensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::WaterLevelPercentCapability>(
            *static_cast<iotsmartsys::core::IWaterLevelSensor *>(cfg.sensor),
            &_eventSink);
    }

    // --------------------------- addWaterLevelLiters ---------------------------
    iotsmartsys::core::WaterLevelLitersCapability *CapabilitiesBuilder::addWaterLevelLiters(const WaterLevelSensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::WaterLevelLitersCapability>(
            *static_cast<iotsmartsys::core::IWaterLevelSensor *>(cfg.sensor),
            &_eventSink);
    }

    // --------------------------- addTemperatureSensor ---------------------------
    iotsmartsys::core::TemperatureSensorCapability *CapabilitiesBuilder::addTemperatureSensor(const TemperatureSensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::TemperatureSensorCapability>(
            name,
            *static_cast<iotsmartsys::core::ITemperatureSensor *>(cfg.sensor),
            &_eventSink, cfg.readIntervalMs);
    }

    // --------------------------- addHumiditySensor ---------------------------
    iotsmartsys::core::HumiditySensorCapability *CapabilitiesBuilder::addHumiditySensor(const HumiditySensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::HumiditySensorCapability>(
            name,
            *static_cast<iotsmartsys::core::IHumiditySensor *>(cfg.sensor),
            &_eventSink);
    }

    // --------------------------- addWaterHeight ---------------------------
    iotsmartsys::core::HeightWaterLevelCapability *CapabilitiesBuilder::addWaterHeight(const WaterLevelSensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::HeightWaterLevelCapability>(
            *static_cast<iotsmartsys::core::IWaterLevelSensor *>(cfg.sensor),
            &_eventSink);
    }

    // --------------------------- addGlpSensor ---------------------------
    iotsmartsys::core::GlpSensorCapability *CapabilitiesBuilder::addGlpSensor(const GlpSensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::GlpSensorCapability>(
            *static_cast<iotsmartsys::core::IGlpSensor *>(cfg.sensor),
            &_eventSink);
    }

    // --------------------------- addOperationalColorSensor ---------------------------
    iotsmartsys::core::OperationalColorSensorCapability *CapabilitiesBuilder::addOperationalColorSensor(const OperationalColorSensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::OperationalColorSensorCapability>(
            *static_cast<iotsmartsys::core::IColorSensor *>(cfg.sensor), &_eventSink, cfg.debounceTimeMs);
    }

    /* addLuminosityCapability */
    iotsmartsys::core::LuminosityCapability *CapabilitiesBuilder::addLuminosityCapability(const LuminositySensorConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        auto name = cfg.capability_name ? std::string(cfg.capability_name) : std::string();
        return createCapability<iotsmartsys::core::LuminosityCapability>(
            name,
            *static_cast<iotsmartsys::core::ILuminositySensor *>(cfg.sensor),
            &_eventSink, cfg.variationTolerance, cfg.readIntervalMs);
    }

    // --------------------------- addGlpMeter ---------------------------
    iotsmartsys::core::GlpMeterPercentCapability *CapabilitiesBuilder::addGlpMeterPercent(const GlpMeterConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::GlpMeterPercentCapability>(
            *static_cast<iotsmartsys::core::IGlpMeter *>(cfg.sensor),
            &_eventSink, cfg.maxKgExpected);
    }

    // --------------------------- addGlpMeterKg ---------------------------
    iotsmartsys::core::GlpMeterKgCapability *CapabilitiesBuilder::addGlpMeterKg(const GlpMeterConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        return createCapability<iotsmartsys::core::GlpMeterKgCapability>(
            *static_cast<iotsmartsys::core::IGlpMeter *>(cfg.sensor),
            &_eventSink);
    }

} // namespace iotsmartsys::app
