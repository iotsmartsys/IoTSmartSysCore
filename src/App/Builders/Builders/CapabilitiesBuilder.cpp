#include "CapabilitiesBuilder.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.h"

#include <cstring>
#include <cmath>

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
        _analogSensorPinCount = 0;
        _currentSensorIdCount = 0;
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
        _analogSensorPinCount = 0;
        _currentSensorIdCount = 0;
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

    bool CapabilitiesBuilder::resolveIdentity(const char *configuredName, const char *type, std::string &outName)
    {
        if (_count >= _capsMax)
        {
            return false;
        }

        auto *logger = iotsmartsys::core::ServiceProvider::instance().logger();

        const size_t typeLen = type ? std::strlen(type) : 0;
        if (typeLen == 0 || typeLen > iotsmartsys::core::kMaxCapabilityTypeBytes)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER", "Rejected capability type of %u bytes (limit %u); nothing was registered.",
                              static_cast<unsigned>(typeLen),
                              static_cast<unsigned>(iotsmartsys::core::kMaxCapabilityTypeBytes));
            }
            return false;
        }

        // An omitted name keeps the vigent automatic generation; the limit is
        // then checked over the generated definitive name.
        outName = (configuredName && *configuredName)
                      ? std::string(configuredName)
                      : (_deviceIdentityProvider.getDeviceID() + "_" + std::string(type));

        if (outName.empty() || outName.size() > iotsmartsys::core::kMaxCapabilityNameBytes)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER", "Rejected capability name of %u bytes for type '%s' (limit %u); nothing was registered.",
                              static_cast<unsigned>(outName.size()), type,
                              static_cast<unsigned>(iotsmartsys::core::kMaxCapabilityNameBytes));
            }
            outName.clear();
            return false;
        }

        for (size_t i = 0; i < _currentSensorIdCount; ++i)
        {
            if (_currentSensorIds[i] == outName)
            {
                if (logger)
                {
                    logger->error("CAP_BUILDER",
                                  "Rejected capability name '%s': it is already used by a current sensor; nothing was registered.",
                                  outName.c_str());
                }
                outName.clear();
                return false;
            }
        }

        return true;
    }

    bool CapabilitiesBuilder::registerCapability(ICapability *cap, void (*destructor)(void *))
    {
        if (_count >= _capsMax)
        {
            return false;
        }

        // BCS-002/BCS-DEC-006: the definitive identity is resolved, validated
        // and fixed here, before the capability occupies a slot. After this
        // point neither capability_name nor type can change.
        std::string definitiveName;
        if (!resolveIdentity(cap->capability_name.empty() ? nullptr : cap->capability_name.c_str(),
                             cap->type.c_str(),
                             definitiveName))
        {
            return false;
        }
        cap->finalizeIdentity(definitiveName.c_str());

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

    bool CapabilitiesBuilder::capabilityIdentityInUse(const std::string &name) const
    {
        for (size_t i = 0; i < _count; ++i)
        {
            if (_caps[i] && _caps[i]->capability_name == name)
            {
                return true;
            }
        }
        return false;
    }

    bool CapabilitiesBuilder::analogSensorPinInUse(int pin) const
    {
        for (size_t i = 0; i < _analogSensorPinCount; ++i)
        {
            if (_analogSensorPins[i] == pin)
            {
                return true;
            }
        }
        return false;
    }

    bool CapabilitiesBuilder::validateCurrentSensorConfig(const iotsmartsys::core::CurrentSensorConfig &cfg) const
    {
        auto *logger = iotsmartsys::core::ServiceProvider::instance().logger();
        auto reject = [logger, &cfg](const char *reason)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER",
                              "Current sensor '%s' rejected: %s; nothing was registered.",
                              cfg.id.c_str(),
                              reason);
            }
            return false;
        };

        if (!_factory.currentSensorTargetSupported())
            return reject("target is not ESP32 classic");
        if (cfg.id.empty())
            return reject("id is required");
        if (cfg.id.size() > iotsmartsys::core::kMaxCapabilityNameBytes ||
            cfg.id.find('\0') != std::string::npos)
            return reject("id is not a valid public capability identity");
        if (cfg.adcResolutionBits != 12 ||
            cfg.adcAttenuation != iotsmartsys::core::CurrentSensorAdcAttenuation::FULL_RANGE)
            return reject("unsupported ADC profile");
        if (cfg.adcMinimumMv < 0.0f || cfg.adcMaximumMv <= cfg.adcMinimumMv)
            return reject("invalid ADC limits");
        if (cfg.supplyNominalMv <= 0.0f ||
            cfg.supplyValidMinimumMv <= 0.0f ||
            cfg.supplyValidMaximumMv < cfg.supplyValidMinimumMv ||
            cfg.supplyNominalMv < cfg.supplyValidMinimumMv ||
            cfg.supplyNominalMv > cfg.supplyValidMaximumMv)
            return reject("invalid supply profile");
        if (cfg.outputToAdcRatio <= 0.0f || cfg.supplyMonitorToVccRatio <= 0.0f)
            return reject("invalid voltage ratio");
        if (cfg.nominalZeroAdcMv <= cfg.adcMinimumMv ||
            cfg.nominalZeroAdcMv >= cfg.adcMaximumMv ||
            cfg.sensitivityAdcMvPerA <= 0.0f || cfg.maximumZeroDeviationMv < 0.0f)
            return reject("invalid zero or sensitivity");
        if (cfg.polarity != 1.0f && cfg.polarity != -1.0f)
            return reject("polarity must be +1 or -1");
        if (cfg.zeroCalibrationSamples == 0 || cfg.samplesPerReading == 0 ||
            cfg.sampleIntervalUs == 0 || cfg.readingIntervalMs == 0 ||
            cfg.capabilityEvaluationIntervalMs == 0)
            return reject("sampling parameters must be positive");
        if (!(cfg.lowPassAlpha > 0.0f && cfg.lowPassAlpha <= 1.0f))
            return reject("lowPassAlpha must be in (0,1]");
        if (cfg.deadbandA < 0.0f || cfg.minimumReportableA < 0.0f ||
            cfg.calibratedMinimumA < 0.0f ||
            cfg.calibratedMaximumA < cfg.calibratedMinimumA ||
            cfg.physicalMinimumA >= 0.0f || cfg.physicalMaximumA <= 0.0f ||
            cfg.calibratedMaximumA > std::fabs(cfg.physicalMinimumA) ||
            cfg.calibratedMaximumA > cfg.physicalMaximumA)
            return reject("invalid current ranges");
        if (cfg.maximumAbsoluteErrorA < 0.0f || cfg.maximumRelativeErrorPercent < 0.0f)
            return reject("invalid accuracy limits");
        if (!_factory.currentSensorPinHasAdc(cfg.adcPin))
            return reject("adcPin has no ADC capability");
        if (_factory.currentSensorPinReserved(cfg.adcPin))
            return reject("adcPin is reserved by the target runtime");
        if (cfg.supplyMonitorAdcPin >= 0)
        {
            if (cfg.supplyMonitorAdcPin == cfg.adcPin)
                return reject("adcPin and supplyMonitorAdcPin must differ");
            if (!_factory.currentSensorPinHasAdc(cfg.supplyMonitorAdcPin))
                return reject("supplyMonitorAdcPin has no ADC capability");
            if (_factory.currentSensorPinReserved(cfg.supplyMonitorAdcPin))
                return reject("supplyMonitorAdcPin is reserved by the target runtime");
        }
        if (analogSensorPinInUse(cfg.adcPin) ||
            (cfg.supplyMonitorAdcPin >= 0 && analogSensorPinInUse(cfg.supplyMonitorAdcPin)))
            return reject("ADC GPIO is already used by another current or voltage sensor");
        if (capabilityIdentityInUse(cfg.id))
            return reject("capability id is already registered");
        return true;
    }

    iotsmartsys::core::CurrentSensorCapability *CapabilitiesBuilder::addCurrentSensor(
        const iotsmartsys::core::CurrentSensorConfig &cfg)
    {
        if (!validateCurrentSensorConfig(cfg))
        {
            return nullptr;
        }

        auto *logger = iotsmartsys::core::ServiceProvider::instance().logger();
        auto fail = [logger, &cfg](const char *reason)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER",
                              "Current sensor '%s' registration failed: %s; nothing was registered.",
                              cfg.id.c_str(),
                              reason);
            }
            return static_cast<iotsmartsys::core::CurrentSensorCapability *>(nullptr);
        };
        if (_count >= _capsMax)
            return fail("capability slots exhausted");
        if (_adaptersCount >= _adaptersMax)
            return fail("adapter slots exhausted");

        std::string name;
        if (!resolveIdentity(cfg.id.c_str(), CURRENT_SENSOR_TYPE, name))
        {
            return nullptr;
        }

        const size_t originalArenaOffset = _arenaOffset;
        const size_t adapterSize = _factory.currentSensorAdapterSize();
        const size_t adapterAlign = _factory.currentSensorAdapterAlign();
        auto adapterDtor = _factory.currentSensorAdapterDestructor();
        if (adapterSize == 0 || adapterAlign == 0 || !adapterDtor)
        {
            return fail("current sensor factory is unavailable");
        }

        void *adapterMemory = allocateAligned(adapterSize, adapterAlign);
        if (!adapterMemory)
        {
            return fail("arena has insufficient space for adapter");
        }
        auto *sensor = _factory.createCurrentSensor(adapterMemory, cfg);
        if (!sensor)
        {
            _arenaOffset = originalArenaOffset;
            return fail("adapter construction failed");
        }

        void *capabilityMemory = allocateAligned(sizeof(iotsmartsys::core::CurrentSensorCapability),
                                                 alignof(iotsmartsys::core::CurrentSensorCapability));
        if (!capabilityMemory)
        {
            adapterDtor(sensor);
            _arenaOffset = originalArenaOffset;
            return fail("arena has insufficient space for capability");
        }
        auto *capability = new (capabilityMemory) iotsmartsys::core::CurrentSensorCapability(
            name, *sensor, &_eventSink, cfg.capabilityEvaluationIntervalMs);
        auto capabilityDtor = [](void *p)
        {
            static_cast<iotsmartsys::core::CurrentSensorCapability *>(p)->~CurrentSensorCapability();
        };

        if (!registerAdapter(sensor, adapterDtor))
        {
            capabilityDtor(capability);
            adapterDtor(sensor);
            _arenaOffset = originalArenaOffset;
            return fail("adapter slot registration failed");
        }
        if (!registerCapability(capability, capabilityDtor))
        {
            _adaptersCount--;
            _adapters[_adaptersCount] = nullptr;
            _adapterDestructors[_adaptersCount] = nullptr;
            capabilityDtor(capability);
            adapterDtor(sensor);
            _arenaOffset = originalArenaOffset;
            return fail("capability slot registration failed");
        }

        _analogSensorPins[_analogSensorPinCount++] = cfg.adcPin;
        if (cfg.supplyMonitorAdcPin >= 0)
        {
            _analogSensorPins[_analogSensorPinCount++] = cfg.supplyMonitorAdcPin;
        }
        _currentSensorIds[_currentSensorIdCount++] = cfg.id;
        return capability;
    }

    bool CapabilitiesBuilder::validateVoltageSensorConfig(
        const iotsmartsys::core::VoltageSensorConfig &cfg) const
    {
        auto *logger = iotsmartsys::core::ServiceProvider::instance().logger();
        auto reject = [logger, &cfg](const char *reason)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER",
                              "Voltage sensor '%s' rejected: %s; nothing was registered.",
                              cfg.id.c_str(),
                              reason);
            }
            return false;
        };

        if (!_factory.voltageSensorTargetSupported())
            return reject("target is not ESP32 classic");
        if (cfg.id.empty())
            return reject("id is required");
        if (cfg.id.size() > iotsmartsys::core::kMaxCapabilityNameBytes ||
            cfg.id.find('\0') != std::string::npos)
            return reject("id is not a valid public capability identity");
        if (cfg.adcResolutionBits != 12 ||
            cfg.adcAttenuation != iotsmartsys::core::VoltageSensorAdcAttenuation::FULL_RANGE)
            return reject("unsupported ADC profile");
        if (!std::isfinite(cfg.adcMinimumMv) || !std::isfinite(cfg.adcMaximumMv) ||
            cfg.adcMinimumMv < 0.0f || cfg.adcMaximumMv <= cfg.adcMinimumMv)
            return reject("invalid ADC limits");
        if (!std::isfinite(cfg.r1Ohms) || !std::isfinite(cfg.r2Ohms) ||
            cfg.r1Ohms <= 0.0f || cfg.r2Ohms <= 0.0f)
            return reject("R1 and R2 must be finite and positive");
        const double dividerRatio =
            (static_cast<double>(cfg.r1Ohms) + cfg.r2Ohms) / cfg.r2Ohms;
        if (!std::isfinite(dividerRatio))
            return reject("resistive divider ratio is not finite");
        if (cfg.samplesPerReading == 0 || cfg.sampleIntervalUs == 0 ||
            cfg.readingIntervalMs == 0 || cfg.capabilityEvaluationIntervalMs == 0)
            return reject("sampling parameters must be positive");
        if (!_factory.voltageSensorPinHasAdc(cfg.adcPin))
            return reject("adcPin has no ADC capability");
        if (_factory.voltageSensorPinReserved(cfg.adcPin))
            return reject("adcPin is reserved by the target runtime");
        if (analogSensorPinInUse(cfg.adcPin))
            return reject("ADC GPIO is already used by another current or voltage sensor");
        if (capabilityIdentityInUse(cfg.id))
            return reject("capability id is already registered");
        return true;
    }

    iotsmartsys::core::VoltageSensorCapability *CapabilitiesBuilder::addVoltageSensor(
        const iotsmartsys::core::VoltageSensorConfig &cfg)
    {
        if (!validateVoltageSensorConfig(cfg))
        {
            return nullptr;
        }

        auto *logger = iotsmartsys::core::ServiceProvider::instance().logger();
        auto fail = [logger, &cfg](const char *reason)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER",
                              "Voltage sensor '%s' registration failed: %s; nothing was registered.",
                              cfg.id.c_str(),
                              reason);
            }
            return static_cast<iotsmartsys::core::VoltageSensorCapability *>(nullptr);
        };
        if (_count >= _capsMax)
            return fail("capability slots exhausted");
        if (_adaptersCount >= _adaptersMax)
            return fail("adapter slots exhausted");

        std::string name;
        if (!resolveIdentity(cfg.id.c_str(), VOLTAGE_SENSOR_TYPE, name))
        {
            return nullptr;
        }

        const size_t originalArenaOffset = _arenaOffset;
        const size_t adapterSize = _factory.voltageSensorAdapterSize();
        const size_t adapterAlign = _factory.voltageSensorAdapterAlign();
        auto adapterDtor = _factory.voltageSensorAdapterDestructor();
        if (adapterSize == 0 || adapterAlign == 0 || !adapterDtor)
        {
            return fail("voltage sensor factory is unavailable");
        }

        void *adapterMemory = allocateAligned(adapterSize, adapterAlign);
        if (!adapterMemory)
        {
            return fail("arena has insufficient space for adapter");
        }
        auto *sensor = _factory.createVoltageSensor(adapterMemory, cfg);
        if (!sensor)
        {
            _arenaOffset = originalArenaOffset;
            return fail("adapter construction failed");
        }

        void *capabilityMemory = allocateAligned(sizeof(iotsmartsys::core::VoltageSensorCapability),
                                                 alignof(iotsmartsys::core::VoltageSensorCapability));
        if (!capabilityMemory)
        {
            adapterDtor(sensor);
            _arenaOffset = originalArenaOffset;
            return fail("arena has insufficient space for capability");
        }
        auto *capability = new (capabilityMemory) iotsmartsys::core::VoltageSensorCapability(
            name, *sensor, &_eventSink, cfg.capabilityEvaluationIntervalMs);
        auto capabilityDtor = [](void *p)
        {
            static_cast<iotsmartsys::core::VoltageSensorCapability *>(p)->~VoltageSensorCapability();
        };

        if (!registerAdapter(sensor, adapterDtor))
        {
            capabilityDtor(capability);
            adapterDtor(sensor);
            _arenaOffset = originalArenaOffset;
            return fail("adapter slot registration failed");
        }
        if (!registerCapability(capability, capabilityDtor))
        {
            _adaptersCount--;
            _adapters[_adaptersCount] = nullptr;
            _adapterDestructors[_adaptersCount] = nullptr;
            capabilityDtor(capability);
            adapterDtor(sensor);
            _arenaOffset = originalArenaOffset;
            return fail("capability slot registration failed");
        }

        _analogSensorPins[_analogSensorPinCount++] = cfg.adcPin;
        return capability;
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
        std::string name;
        if (!resolveIdentity(cfg.capability_name, LIGHT_ACTUATOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::LightCapability>(
            name,
            *hardwareAdapter, &_eventSink);
    }

    // --------------------------- addSwitch ---------------------------
    iotsmartsys::core::SwitchCapability *CapabilitiesBuilder::addSwitch(const SwitchConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, SWITCH_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::SwitchCapability>(
            name,
            *hardwareAdapter, &_eventSink);
    }

    // --------------------------- addFan ---------------------------
    iotsmartsys::core::FanCapability *CapabilitiesBuilder::addFan(const FanConfig &cfg)
    {
        auto *logger = iotsmartsys::core::ServiceProvider::instance().logger();
        auto fail = [logger](const char *reason)
        {
            if (logger)
            {
                logger->error("CAP_BUILDER",
                              "Fan capability registration failed: %s; nothing was registered.",
                              reason);
            }
            return static_cast<iotsmartsys::core::FanCapability *>(nullptr);
        };

        std::string name;
        if (!resolveIdentity(cfg.capability_name, FAN_ACTUATOR_TYPE, name))
            return nullptr;
        if (capabilityIdentityInUse(name))
            return fail("capability identity is already registered");
        if (_adaptersCount >= _adaptersMax)
            return fail("adapter slots exhausted");

        const size_t originalArenaOffset = _arenaOffset;
        const size_t adapterSize = _factory.outputAdapterSize();
        const size_t adapterAlign = _factory.outputAdapterAlign();
        auto adapterDtor = _factory.outputAdapterDestructor();
        if (adapterSize == 0 || adapterAlign == 0 || !adapterDtor)
            return fail("output adapter factory is unavailable");

        void *adapterMemory = allocateAligned(adapterSize, adapterAlign);
        if (!adapterMemory)
            return fail("arena has insufficient space for adapter");

        auto *hardwareAdapter = _factory.createOutput(adapterMemory, cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
        {
            _arenaOffset = originalArenaOffset;
            return fail("output adapter construction failed");
        }

        void *capabilityMemory = allocateAligned(sizeof(iotsmartsys::core::FanCapability),
                                                 alignof(iotsmartsys::core::FanCapability));
        if (!capabilityMemory)
        {
            adapterDtor(hardwareAdapter);
            _arenaOffset = originalArenaOffset;
            return fail("arena has insufficient space for capability");
        }

        auto *capability = new (capabilityMemory) iotsmartsys::core::FanCapability(
            name, *hardwareAdapter, &_eventSink);
        auto capabilityDtor = [](void *p)
        {
            static_cast<iotsmartsys::core::FanCapability *>(p)->~FanCapability();
        };

        if (!registerAdapter(hardwareAdapter, adapterDtor))
        {
            capabilityDtor(capability);
            adapterDtor(hardwareAdapter);
            _arenaOffset = originalArenaOffset;
            return fail("adapter slot registration failed");
        }
        if (!registerCapability(capability, capabilityDtor))
        {
            _adaptersCount--;
            _adapters[_adaptersCount] = nullptr;
            _adapterDestructors[_adaptersCount] = nullptr;
            capabilityDtor(capability);
            adapterDtor(hardwareAdapter);
            _arenaOffset = originalArenaOffset;
            return fail("capability slot registration failed");
        }

        return capability;
    }

    // --------------------------- addPushButton ---------------------------
    iotsmartsys::core::PushButtonCapability *CapabilitiesBuilder::addPushButton(const PushButtonConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, PUSH_BUTTON_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::PushButtonCapability>(
            name,
            *hardwareAdapter,
            &_eventSink,
            static_cast<unsigned long>(cfg.debounceTimeMs));
    }

    // --------------------------- addTouchButton ---------------------------
    iotsmartsys::core::TouchButtonCapability *CapabilitiesBuilder::addTouchButton(const TouchButtonConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, BUTTON_TOUCH_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::TouchButtonCapability>(
            name,
            *hardwareAdapter,
            &_eventSink,
            static_cast<unsigned long>(cfg.debounceTimeMs));
    }

    // --------------------------- addValve ---------------------------
    iotsmartsys::core::ValveCapability *CapabilitiesBuilder::addValve(const ValveConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, VALVE_ACTUATOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

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
        std::string name;
        if (!resolveIdentity(cfg.capability_name, LED_ACTUATOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::LEDCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addAlarm ---------------------------

    iotsmartsys::core::AlarmCapability *CapabilitiesBuilder::addAlarm(const AlarmConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, ALARM_ACTUATOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::AlarmCapability>(
            name,
            cfg.ringDurationMs,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addDoorSensor ---------------------------
    iotsmartsys::core::DoorSensorCapability *CapabilitiesBuilder::addDoorSensor(const DoorSensorConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, DOOR_SENSOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO, iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON, iotsmartsys::core::InputPullMode::PULL_UP);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::DoorSensorCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addPirSensor ---------------------------
    iotsmartsys::core::PirSensorCapability *CapabilitiesBuilder::addPirSensor(const PirSensorConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, PIR_SENSOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO, cfg.highIsOn ? iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON : iotsmartsys::core::HardwareDigitalLogic::LOW_IS_ON, iotsmartsys::core::InputPullMode::NONE);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::PirSensorCapability>(
            name,
            *hardwareAdapter,
            &_eventSink,
            cfg.debounceTimeMs);
    }

    // --------------------------- addClapSensor ---------------------------
    iotsmartsys::core::ClapSensorCapability *CapabilitiesBuilder::addClapSensor(const ClapSensorConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, CLAP_SENSOR_TYPE, name))
            return nullptr;

        const auto logic = cfg.highIsOn ? iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON
                                        : iotsmartsys::core::HardwareDigitalLogic::LOW_IS_ON;
        const auto pullMode = cfg.highIsOn ? iotsmartsys::core::InputPullMode::PULL_DOWN
                                           : iotsmartsys::core::InputPullMode::PULL_UP;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO, logic, pullMode);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::ClapSensorCapability>(
            name.c_str(),
            *hardwareAdapter,
            &_eventSink,
            cfg.debounceTimeMs);
    }

    // --------------------------- addSwitchPlug ---------------------------
    iotsmartsys::core::SwitchPlugCapability *CapabilitiesBuilder::addSwitchPlug(const SwitchConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, SWITCH_PLUG_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createOutputAdapter(cfg.GPIO, cfg.highIsOn);
        if (!hardwareAdapter)
            return nullptr;

        return createCapability<iotsmartsys::core::SwitchPlugCapability>(
            name,
            *hardwareAdapter,
            &_eventSink);
    }

    // --------------------------- addWaterFlowHallSensor ---------------------------
    iotsmartsys::core::WaterFlowHallSensorCapability *CapabilitiesBuilder::addWaterFlowHallSensor(const WaterFlowHallSensorConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, WATER_FLOW_SENSOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapter = createInputAdapter(cfg.GPIO);
        if (!hardwareAdapter)
            return nullptr;

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

        std::string name;
        if (!resolveIdentity(cfg.capability_name, TEMPERATURE_SENSOR_TYPE, name))
            return nullptr;

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

        std::string name;
        if (!resolveIdentity(cfg.capability_name, HUMIDITY_SENSOR_TYPE, name))
            return nullptr;

        return createCapability<iotsmartsys::core::HumiditySensorCapability>(
            name,
            *static_cast<iotsmartsys::core::IHumiditySensor *>(cfg.sensor),
            &_eventSink);
    }

    // --------------------------- addDistance ---------------------------
    iotsmartsys::core::DistanceCapability *CapabilitiesBuilder::addDistance(const DistanceCapabilityConfig &cfg)
    {
        if (!cfg.sensor)
            return nullptr;

        std::string name;
        if (!resolveIdentity(cfg.capability_name, DISTANCE_SENSOR_TYPE, name))
            return nullptr;

        return createCapability<iotsmartsys::core::DistanceCapability>(
            name,
            *static_cast<iotsmartsys::core::IDistanceSensor *>(cfg.sensor),
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

        std::string name;
        if (!resolveIdentity(cfg.capability_name, LIGHT_SENSOR_TYPE, name))
            return nullptr;

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

    // --------------------------- addGarageControlCapability ---------------------------
    iotsmartsys::core::GarageControlCapability *CapabilitiesBuilder::addGarageControlCapability(const GarageControlConfig &cfg)
    {
        std::string name;
        if (!resolveIdentity(cfg.capability_name, GARAGE_ACTUATOR_TYPE, name))
            return nullptr;

        auto *hardwareAdapterOpen = createOutputAdapter(cfg.GPIO_OPEN, true);
        if (!hardwareAdapterOpen)
            return nullptr;

        auto *hardwareAdapterClose = createOutputAdapter(cfg.GPIO_CLOSE, true);
        if (!hardwareAdapterClose)
            return nullptr;

        auto *hardwareAdapterLock = createOutputAdapter(cfg.GPIO_LOCK, true);
        if (!hardwareAdapterLock)
            return nullptr;

        auto *hardwareAdapterStopUnlock = createOutputAdapter(cfg.GPIO_STOP_UNLOCK, true);
        if (!hardwareAdapterStopUnlock)
            return nullptr;

        iotsmartsys::core::IInputHardwareAdapter *hardwareAdapterOpenSensor = nullptr;
        if (cfg.GPIO_OPEN_SENSOR > -1)
        {
            hardwareAdapterOpenSensor = createInputAdapter(cfg.GPIO_OPEN_SENSOR, iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON, iotsmartsys::core::InputPullMode::PULL_UP);
            if (!hardwareAdapterOpenSensor)
                return nullptr;
        }

        iotsmartsys::core::IInputHardwareAdapter *hardwareAdapterCloseSensor = nullptr;
        if (cfg.GPIO_CLOSE_SENSOR > -1)
        {
            hardwareAdapterCloseSensor = createInputAdapter(cfg.GPIO_CLOSE_SENSOR, iotsmartsys::core::HardwareDigitalLogic::HIGH_IS_ON, iotsmartsys::core::InputPullMode::PULL_UP);
            if (!hardwareAdapterCloseSensor)
                return nullptr;
        }

        return createCapability<iotsmartsys::core::GarageControlCapability>(
            name,
            cfg.debounceTimeMs,
            *hardwareAdapterOpen,
            *hardwareAdapterClose,
            *hardwareAdapterStopUnlock,
            *hardwareAdapterLock,
            hardwareAdapterOpenSensor,
            hardwareAdapterCloseSensor,
            &_eventSink,
            cfg.sensorDebounceTimeMs);
    }

} // namespace iotsmartsys::app
