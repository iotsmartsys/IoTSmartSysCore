#include <Arduino.h>
#include <unity.h>

#include <cmath>
#include <cstddef>
#include <limits>

#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Config/BuildConfig.h"
#include "Contracts/Capabilities/PowerEnergyCapability.h"
#include "Contracts/Providers/Time.h"
#include "Core/Sensors/CompositePowerSensor.h"
#include "Platform/Arduino/Sensors/INA3221PowerSensor.h"
#include "../test_capability_identity/mocks/FakeAdapterFactory.h"

using namespace iotsmartsys;

namespace
{
    class FakeTime final : public core::ITimeProvider
    {
    public:
        std::uint64_t nowMs() const override { return now; }
        std::uint64_t now{0};
    };

    class Sink final : public core::ICapabilityEventSink
    {
    public:
        void onStateChanged(const core::CapabilityStateChanged &event) override
        {
            calls++;
            last = event;
        }
        int calls{0};
        core::CapabilityStateChanged last;
    };

    class FakePowerSensor final : public core::IPowerSensor
    {
    public:
        void setup() override { setupCalls++; }
        void handle() override { handleCalls++; }
        long lastStateReadMillis() const override { return timestamp; }
        const core::PowerMeasurement &powerMeasurement() const override
        {
            readCalls++;
            return measurement;
        }

        mutable int readCalls{0};
        int setupCalls{0};
        int handleCalls{0};
        long timestamp{0};
        core::PowerMeasurement measurement;
    };

    class LifetimePowerSensor final : public core::IPowerSensor
    {
    public:
        explicit LifetimePowerSensor(bool &destroyed) : _destroyed(destroyed) {}
        ~LifetimePowerSensor() override { _destroyed = true; }
        void setup() override {}
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }
        const core::PowerMeasurement &powerMeasurement() const override { return _measurement; }

    private:
        bool &_destroyed;
        core::PowerMeasurement _measurement;
    };

    class FakeVoltageSensor final : public core::IVoltageSensor
    {
    public:
        void setup() override { setupCalls++; }
        void handle() override { handleCalls++; }
        long lastStateReadMillis() const override { return 0; }
        const core::VoltageMeasurement &voltageMeasurement() const override
        {
            readCalls++;
            return measurement;
        }
        mutable int readCalls{0};
        int setupCalls{0};
        int handleCalls{0};
        core::VoltageMeasurement measurement;
    };

    class FakeCurrentSensor final : public core::ICurrentSensor
    {
    public:
        void setup() override { setupCalls++; }
        void handle() override { handleCalls++; }
        long lastStateReadMillis() const override { return 0; }
        const core::CurrentMeasurement &currentMeasurement() const override
        {
            readCalls++;
            return measurement;
        }
        std::optional<float> calibratedZeroAdcMv() const override { return std::nullopt; }
        void requestZeroCalibration() override {}
        mutable int readCalls{0};
        int setupCalls{0};
        int handleCalls{0};
        core::CurrentMeasurement measurement;
    };

    class ControlledINA3221PowerSensor final : public platform::arduino::INA3221PowerSensor
    {
    public:
        ControlledINA3221PowerSensor(platform::arduino::INA3221Device &device,
                                    const platform::arduino::INA3221PowerSensorConfig &config)
            : INA3221PowerSensor(device, config) {}

        bool setupResult{true};
        bool available{true};
        float voltageV{24.0f};
        float currentA{2.0f};
        std::uint32_t nowMs{0};

    protected:
        bool setupDevice() override { return setupResult; }
        bool deviceAvailable() const override { return available; }
        float readBusVoltage(std::uint8_t) override { return voltageV; }
        float readCurrentAmps(std::uint8_t) override { return currentA; }
        std::uint32_t nowMillis() const override { return nowMs; }
    };

    FakeTime fakeTime;

    platform::arduino::INA3221PowerSensorConfig validINAConfig()
    {
        platform::arduino::INA3221PowerSensorConfig config;
        config.channel = 0;
        config.shuntResistanceOhms = 0.1f;
        config.polarity = 1.0f;
        config.deadbandA = 0.01f;
        config.minimumReportableA = 0.02f;
        config.maximumAbsoluteCurrentA = 1.638f;
        config.minimumVoltageV = 0.0f;
        config.maximumVoltageV = 26.0f;
        config.readingIntervalMs = 1;
        return config;
    }
}

void setUp()
{
    fakeTime.now = 0;
    core::Time::setProvider(&fakeTime);
}

void tearDown() {}

// PWR-AC-001/PWR-AC-005: capability owns sensor lifecycle, consumes one
// snapshot per eligible evaluation and keeps trapezoidal energy semantics.
void test_capability_drives_power_sensor_and_integrates_energy()
{
    FakePowerSensor sensor;
    Sink sink;
    core::PowerEnergyCapability capability("power", sensor, &sink, 1000);
    sensor.measurement.powerW = 100.0;
    sensor.measurement.measurementStatus = core::PowerMeasurementStatus::VALID;

    capability.setup();
    capability.handle();
    TEST_ASSERT_EQUAL(1, sensor.setupCalls);
    TEST_ASSERT_EQUAL(1, sensor.handleCalls);
    TEST_ASSERT_EQUAL(1, sensor.readCalls);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 0.0, capability.powerEnergyMeasurement().energyWh);

    fakeTime.now = 500;
    capability.handle();
    TEST_ASSERT_EQUAL(2, sensor.handleCalls);
    TEST_ASSERT_EQUAL(1, sensor.readCalls);

    fakeTime.now = 3600;
    sensor.measurement.powerW = 200.0;
    capability.handle();
    TEST_ASSERT_EQUAL(3, sensor.handleCalls);
    TEST_ASSERT_EQUAL(2, sensor.readCalls);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 0.15, capability.powerEnergyMeasurement().energyWh);
}

// PWR-AC-001: incoherent and negative sensor snapshots are rejected.
void test_capability_rejects_incoherent_power_snapshot()
{
    FakePowerSensor sensor;
    core::PowerEnergyCapability capability("power", sensor, nullptr, 1);
    sensor.measurement.powerW = -1.0;
    sensor.measurement.measurementStatus = core::PowerMeasurementStatus::VALID;
    capability.setup();
    capability.handle();
    TEST_ASSERT_EQUAL(core::PowerEnergyMeasurementStatus::INPUT_INVALID,
                      capability.powerEnergyMeasurement().measurementStatus);
    TEST_ASSERT_FALSE(capability.powerEnergyMeasurement().powerW.has_value());
}

// PWR-AC-001/PWR-AC-005: cadence and integration use modular timestamps, and
// reset clears the baseline without touching the sensor.
void test_capability_tolerates_rollover_and_reset_is_local()
{
    FakePowerSensor sensor;
    core::PowerEnergyCapability capability("power", sensor, nullptr, 1000);
    sensor.measurement.powerW = 100.0;
    sensor.measurement.measurementStatus = core::PowerMeasurementStatus::VALID;
    const std::uint64_t rollover = std::numeric_limits<std::uint32_t>::max();
    fakeTime.now = rollover - 500U;
    capability.setup();
    capability.handle();

    fakeTime.now = rollover + 501U;
    capability.handle();
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 100.0 * 1001.0 / 3600000.0,
                              capability.powerEnergyMeasurement().energyWh);

    const int readsBeforeReset = sensor.readCalls;
    const int handlesBeforeReset = sensor.handleCalls;
    capability.resetEnergy();
    TEST_ASSERT_EQUAL(readsBeforeReset, sensor.readCalls);
    TEST_ASSERT_EQUAL(handlesBeforeReset, sensor.handleCalls);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 0.0, capability.powerEnergyMeasurement().energyWh);
}

// PWR-AC-002: composition preserves magnitude and never propagates lifecycle.
void test_composite_calculates_magnitude_without_driving_sources()
{
    FakeVoltageSensor voltage;
    FakeCurrentSensor current;
    voltage.measurement.voltageV = 24.0f;
    voltage.measurement.measurementStatus = core::VoltageMeasurementStatus::VALID;
    current.measurement.currentA = -2.0f;
    current.measurement.measurementStatus = core::CurrentMeasurementStatus::VALID;
    current.measurement.supplyStatus = core::CurrentSupplyStatus::IN_RANGE;

    core::CompositePowerSensor composite(voltage, current, 1);
    composite.setup();
    composite.handle();
    TEST_ASSERT_EQUAL(0, voltage.setupCalls);
    TEST_ASSERT_EQUAL(0, voltage.handleCalls);
    TEST_ASSERT_EQUAL(0, current.setupCalls);
    TEST_ASSERT_EQUAL(0, current.handleCalls);
    TEST_ASSERT_EQUAL(1, voltage.readCalls);
    TEST_ASSERT_EQUAL(1, current.readCalls);
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::VALID,
                      composite.powerMeasurement().measurementStatus);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 48.0, *composite.powerMeasurement().powerW);
}

// PWR-AC-002: input-invalid has precedence over a simultaneous not-ready input.
void test_composite_preserves_status_precedence()
{
    FakeVoltageSensor voltage;
    FakeCurrentSensor current;
    voltage.measurement.measurementStatus = core::VoltageMeasurementStatus::ADC_SATURATION;
    current.measurement.measurementStatus = core::CurrentMeasurementStatus::NOT_READY;
    current.measurement.supplyStatus = core::CurrentSupplyStatus::UNKNOWN;
    core::CompositePowerSensor composite(voltage, current, 1);
    composite.setup();
    composite.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::INPUT_INVALID,
                      composite.powerMeasurement().measurementStatus);
}

// PWR-AC-003: INA3221 source calculates power in software and remains estimated.
void test_ina3221_power_is_calculated_and_estimated()
{
    platform::arduino::INA3221Device device(Wire);
    auto config = validINAConfig();
    config.shuntResistanceOhms = 0.05f;
    config.maximumAbsoluteCurrentA = 3.0f;
    ControlledINA3221PowerSensor sensor(device, config);
    sensor.voltageV = 12.0f;
    sensor.currentA = -2.0f;
    sensor.setup();
    sensor.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::ESTIMATED,
                      sensor.powerMeasurement().measurementStatus);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 24.0, *sensor.powerMeasurement().powerW);
}

// PWR-AC-003: invalid config and runtime range failures never expose power.
void test_ina3221_invalid_configuration_and_range_are_observable()
{
    platform::arduino::INA3221Device device(Wire);
    auto invalidConfig = validINAConfig();
    invalidConfig.channel = 3;
    ControlledINA3221PowerSensor invalid(device, invalidConfig);
    invalid.setup();
    invalid.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::NOT_READY,
                      invalid.powerMeasurement().measurementStatus);

    auto config = validINAConfig();
    ControlledINA3221PowerSensor ranged(device, config);
    ranged.currentA = 2.0f;
    ranged.setup();
    ranged.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::INPUT_INVALID,
                      ranged.powerMeasurement().measurementStatus);
    TEST_ASSERT_FALSE(ranged.powerMeasurement().powerW.has_value());
}

// PWR-AC-003: lack of device evidence, non-finite acquisition and shunt setup
// failure remain observable without exposing a numeric power value.
void test_ina3221_not_ready_paths_do_not_expose_power()
{
    platform::arduino::INA3221Device device(Wire);
    auto config = validINAConfig();

    ControlledINA3221PowerSensor unavailable(device, config);
    unavailable.currentA = 1.0f;
    unavailable.setup();
    unavailable.handle();
    TEST_ASSERT_TRUE(unavailable.powerMeasurement().powerW.has_value());
    unavailable.available = false;
    unavailable.nowMs = 1;
    unavailable.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::NOT_READY,
                      unavailable.powerMeasurement().measurementStatus);
    TEST_ASSERT_FALSE(unavailable.powerMeasurement().powerW.has_value());

    ControlledINA3221PowerSensor nonFinite(device, config);
    nonFinite.currentA = std::numeric_limits<float>::quiet_NaN();
    nonFinite.setup();
    nonFinite.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::NOT_READY,
                      nonFinite.powerMeasurement().measurementStatus);
    TEST_ASSERT_FALSE(nonFinite.powerMeasurement().powerW.has_value());

    ControlledINA3221PowerSensor shuntConflict(device, config);
    shuntConflict.setupResult = false;
    shuntConflict.setup();
    shuntConflict.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::NOT_READY,
                      shuntConflict.powerMeasurement().measurementStatus);
    TEST_ASSERT_FALSE(shuntConflict.powerMeasurement().powerW.has_value());
}

// PWR-AC-003: deadband normalizes to zero, while the interval below the
// reportable threshold preserves magnitude; both readings remain estimated.
void test_ina3221_deadband_and_minimum_reportable_boundaries()
{
    platform::arduino::INA3221Device device(Wire);
    auto config = validINAConfig();

    ControlledINA3221PowerSensor deadband(device, config);
    deadband.currentA = 0.005f;
    deadband.setup();
    deadband.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::ESTIMATED,
                      deadband.powerMeasurement().measurementStatus);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 0.0, *deadband.powerMeasurement().powerW);

    ControlledINA3221PowerSensor belowReportable(device, config);
    belowReportable.currentA = 0.015f;
    belowReportable.setup();
    belowReportable.handle();
    TEST_ASSERT_EQUAL(core::PowerMeasurementStatus::ESTIMATED,
                      belowReportable.powerMeasurement().measurementStatus);
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 0.36, *belowReportable.powerMeasurement().powerW);
}

// PWR-AC-005: formatting and event suppression remain representation-based,
// and an invalid snapshot breaks the integration baseline.
void test_capability_preserves_publication_and_invalid_baseline_semantics()
{
    FakePowerSensor sensor;
    Sink sink;
    core::PowerEnergyCapability capability("power", sensor, &sink, 1);
    sensor.measurement.powerW = 0.0;
    sensor.measurement.measurementStatus = core::PowerMeasurementStatus::ESTIMATED;
    capability.setup();
    capability.handle();
    TEST_ASSERT_EQUAL(1, sink.calls);
    TEST_ASSERT_EQUAL_STRING("0.00", sink.last.value.c_str());
    TEST_ASSERT_TRUE(sink.last.energyWh.has_value());
    TEST_ASSERT_EQUAL_STRING("0.000", sink.last.energyWh->c_str());
    TEST_ASSERT_TRUE(sink.last.measurementStatus.has_value());
    TEST_ASSERT_EQUAL_STRING("ESTIMATED", sink.last.measurementStatus->c_str());
    TEST_ASSERT_FALSE(sink.last.supplyStatus.has_value());

    fakeTime.now = 1;
    capability.handle();
    TEST_ASSERT_EQUAL(1, sink.calls);

    sensor.measurement.powerW.reset();
    sensor.measurement.measurementStatus = core::PowerMeasurementStatus::INPUT_INVALID;
    fakeTime.now = 2;
    capability.handle();
    TEST_ASSERT_EQUAL(2, sink.calls);
    TEST_ASSERT_EQUAL_STRING("", sink.last.value.c_str());

    sensor.measurement.powerW = 100.0;
    sensor.measurement.measurementStatus = core::PowerMeasurementStatus::VALID;
    fakeTime.now = 3;
    capability.handle();
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 0.0, capability.powerEnergyMeasurement().energyWh);
}

// PWR-AC-004: the principal overload stores only the capability and never
// registers or destroys the externally owned power sensor.
void test_builder_preserves_external_power_sensor_ownership()
{
    test::mocks::FakeAdapterFactory factory;
    test::mocks::FakeDeviceIdentityProvider identity;
    test::mocks::NoopEventSink eventSink;
    core::ICapability *capSlots[1]{};
    void (*capDtors[1])(void *){};
    void *adapterSlots[1]{};
    void (*adapterDtors[1])(void *){};
    alignas(std::max_align_t) std::uint8_t arena[512]{};
    app::CapabilitiesBuilder builder(factory, eventSink,
                                     capSlots, capDtors, 1,
                                     adapterSlots, adapterDtors, 1,
                                     arena, sizeof(arena), identity);
    bool destroyed = false;
    LifetimePowerSensor sensor(destroyed);
    core::PowerEnergyConfig config{"power_external", 1000};

    TEST_ASSERT_NOT_NULL(builder.addPowerEnergyCapability(config, sensor));
    TEST_ASSERT_EQUAL(1, builder.count());
    TEST_ASSERT_NULL(adapterSlots[0]);
    builder.reset();
    TEST_ASSERT_FALSE(destroyed);
}

// PWR-AC-004: the compatibility overload owns its composite in the builder
// arena and reset releases both registrations while leaving source sensors alone.
void test_builder_owns_composite_for_compatibility_overload()
{
    test::mocks::FakeAdapterFactory factory;
    test::mocks::FakeDeviceIdentityProvider identity;
    test::mocks::NoopEventSink eventSink;
    core::ICapability *capSlots[1]{};
    void (*capDtors[1])(void *){};
    void *adapterSlots[1]{};
    void (*adapterDtors[1])(void *){};
    alignas(std::max_align_t) std::uint8_t arena[512]{};
    app::CapabilitiesBuilder builder(factory, eventSink,
                                     capSlots, capDtors, 1,
                                     adapterSlots, adapterDtors, 1,
                                     arena, sizeof(arena), identity);
    FakeVoltageSensor voltage;
    FakeCurrentSensor current;
    core::PowerEnergyConfig config{"power_composite", 1000};

    TEST_ASSERT_NOT_NULL(builder.addPowerEnergyCapability(config, voltage, current));
    TEST_ASSERT_EQUAL(1, builder.count());
    TEST_ASSERT_NOT_NULL(adapterSlots[0]);
    builder.reset();
    TEST_ASSERT_EQUAL(0, builder.count());
    TEST_ASSERT_NULL(capSlots[0]);
    TEST_ASSERT_NULL(adapterSlots[0]);
    TEST_ASSERT_EQUAL(sizeof(arena), builder.remainingArenaBytes());
}

// PWR-AC-004: if the capability does not fit after the composite, construction
// rolls back the arena and leaves both registration tables untouched.
void test_builder_rolls_back_partial_composite_construction()
{
    test::mocks::FakeAdapterFactory factory;
    test::mocks::FakeDeviceIdentityProvider identity;
    test::mocks::NoopEventSink eventSink;
    core::ICapability *capSlots[1]{};
    void (*capDtors[1])(void *){};
    void *adapterSlots[1]{};
    void (*adapterDtors[1])(void *){};
    alignas(std::max_align_t) std::uint8_t arena[sizeof(core::CompositePowerSensor)]{};
    app::CapabilitiesBuilder builder(factory, eventSink,
                                     capSlots, capDtors, 1,
                                     adapterSlots, adapterDtors, 1,
                                     arena, sizeof(arena), identity);
    FakeVoltageSensor voltage;
    FakeCurrentSensor current;
    core::PowerEnergyConfig config{"power_rollback", 1000};

    TEST_ASSERT_NULL(builder.addPowerEnergyCapability(config, voltage, current));
    TEST_ASSERT_EQUAL(0, builder.count());
    TEST_ASSERT_NULL(capSlots[0]);
    TEST_ASSERT_NULL(adapterSlots[0]);
    TEST_ASSERT_EQUAL(sizeof(arena), builder.remainingArenaBytes());
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_capability_drives_power_sensor_and_integrates_energy);
    RUN_TEST(test_capability_rejects_incoherent_power_snapshot);
    RUN_TEST(test_capability_tolerates_rollover_and_reset_is_local);
    RUN_TEST(test_composite_calculates_magnitude_without_driving_sources);
    RUN_TEST(test_composite_preserves_status_precedence);
    RUN_TEST(test_ina3221_power_is_calculated_and_estimated);
    RUN_TEST(test_ina3221_invalid_configuration_and_range_are_observable);
    RUN_TEST(test_ina3221_not_ready_paths_do_not_expose_power);
    RUN_TEST(test_ina3221_deadband_and_minimum_reportable_boundaries);
    RUN_TEST(test_capability_preserves_publication_and_invalid_baseline_semantics);
    RUN_TEST(test_builder_preserves_external_power_sensor_ownership);
    RUN_TEST(test_builder_owns_composite_for_compatibility_overload);
    RUN_TEST(test_builder_rolls_back_partial_composite_construction);
    UNITY_END();
}

void loop() {}
