#include <Arduino.h>
#include <unity.h>

#include <string>
#include <type_traits>

#include "App/Builders/Builders/CapabilitiesBuilder.h"
#include "Contracts/Capabilities/SwitchCapability.h"
#include "mocks/FakeAdapterFactory.h"

using namespace iotsmartsys;

// Public identity maxima published by BCS-DEC-005.
static const size_t MAX_NAME_BYTES = 63;
static const size_t MAX_TYPE_BYTES = 31;

static test::mocks::FakeAdapterFactory hwFactory;
static test::mocks::FakeDeviceIdentityProvider deviceIdentity;
static test::mocks::NoopEventSink eventSink;

static core::ICapability *capSlots[config::kMaxCapabilities];
static void (*capDtors[config::kMaxCapabilities])(void *);
static void *adapterSlots[config::kMaxAdapters];
static void (*adapterDtors[config::kMaxAdapters])(void *);
static uint8_t arena[config::kCapabilityArenaBytes];

static app::CapabilitiesBuilder *builder = nullptr;
static uint8_t builderStorage[sizeof(app::CapabilitiesBuilder)];

static void rebuildBuilder()
{
    if (builder)
    {
        builder->reset();
        builder->~CapabilitiesBuilder();
    }
    for (size_t i = 0; i < config::kMaxCapabilities; ++i)
    {
        capSlots[i] = nullptr;
        capDtors[i] = nullptr;
        adapterSlots[i] = nullptr;
        adapterDtors[i] = nullptr;
    }
    hwFactory.outputsCreated = 0;
    hwFactory.inputsCreated = 0;
    builder = new (builderStorage) app::CapabilitiesBuilder(
        hwFactory, eventSink,
        capSlots, capDtors, config::kMaxCapabilities,
        adapterSlots, adapterDtors, config::kMaxAdapters,
        arena, sizeof(arena),
        deviceIdentity);
}

void setUp(void) { rebuildBuilder(); }
void tearDown(void) {}

// ---------------------------------------------------------------------------
// BCS-AC-021 — the identity is publicly readable and not publicly assignable,
// and the deprecated rename methods keep their `void` signature.
// ---------------------------------------------------------------------------

// Compile-time oracle: assignment to the identity fields is not part of the
// supported API. If either expression became well-formed again, this detector
// would report true and the test below would fail.
template <typename T, typename = void>
struct IdentityIsAssignable : std::false_type
{
};

template <typename T>
struct IdentityIsAssignable<T, decltype(void(std::declval<T &>().capability_name = std::string("x")))>
    : std::true_type
{
};

template <typename T, typename = void>
struct TypeIsAssignable : std::false_type
{
};

template <typename T>
struct TypeIsAssignable<T, decltype(void(std::declval<T &>().type = std::string("x")))> : std::true_type
{
};

void test_identity_is_readable_but_not_assignable()
{
    app::SwitchConfig cfg;
    cfg.GPIO = 5;
    cfg.capability_name = "dev1_switch_named";

    auto *cap = builder->addSwitch(cfg);
    TEST_ASSERT_NOT_NULL(cap);

    // Public read keeps working exactly as before.
    TEST_ASSERT_EQUAL_STRING("dev1_switch_named", cap->capability_name.c_str());
    TEST_ASSERT_EQUAL_STRING(SWITCH_TYPE, cap->type.c_str());
    TEST_ASSERT_FALSE(cap->capability_name.empty());
    TEST_ASSERT_TRUE(cap->capability_name == std::string("dev1_switch_named"));

    // Public assignment is not offered by the API.
    TEST_ASSERT_FALSE(IdentityIsAssignable<core::SwitchCapability>::value);
    TEST_ASSERT_FALSE(TypeIsAssignable<core::SwitchCapability>::value);

    // The deprecated methods keep returning void.
    TEST_ASSERT_TRUE((std::is_same<decltype(std::declval<core::ICapability &>().rename("x")), void>::value));
    TEST_ASSERT_TRUE((std::is_same<decltype(std::declval<core::ICapability &>().applyRenamedName("x")), void>::value));
}

void test_deprecated_rename_preserves_the_registered_identity()
{
    app::SwitchConfig cfg;
    cfg.GPIO = 5;
    cfg.capability_name = "dev1_switch_named";

    auto *cap = builder->addSwitch(cfg);
    TEST_ASSERT_NOT_NULL(cap);

    core::ICapability *asBase = cap;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    asBase->rename("something_else");
    asBase->applyRenamedName("other_device");
#pragma GCC diagnostic pop

    // Silently preserved: neither name nor type moved.
    TEST_ASSERT_EQUAL_STRING("dev1_switch_named", cap->capability_name.c_str());
    TEST_ASSERT_EQUAL_STRING(SWITCH_TYPE, cap->type.c_str());
}

// ---------------------------------------------------------------------------
// BCS-AC-002 — the definitive name is fixed before registration; omitting it
// keeps the vigent automatic generation.
// ---------------------------------------------------------------------------
void test_omitted_name_uses_automatic_generation()
{
    app::SwitchConfig cfg;
    cfg.GPIO = 5;

    auto *cap = builder->addSwitch(cfg);
    TEST_ASSERT_NOT_NULL(cap);

    const std::string expected = deviceIdentity.deviceId + "_" + SWITCH_TYPE;
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), cap->capability_name.c_str());
}

void test_identities_at_the_public_limits_are_accepted()
{
    const std::string maxName(MAX_NAME_BYTES, 'n');

    app::SwitchConfig cfg;
    cfg.GPIO = 5;
    cfg.capability_name = maxName.c_str();

    auto *cap = builder->addSwitch(cfg);
    TEST_ASSERT_NOT_NULL(cap);
    TEST_ASSERT_EQUAL(MAX_NAME_BYTES, cap->capability_name.size());
    TEST_ASSERT_EQUAL_STRING(maxName.c_str(), cap->capability_name.c_str());

    // Every concrete binary type stays within the 31-byte type limit.
    TEST_ASSERT_LESS_OR_EQUAL(MAX_TYPE_BYTES, strlen(SWITCH_TYPE));
    TEST_ASSERT_LESS_OR_EQUAL(MAX_TYPE_BYTES, strlen(SWITCH_PLUG_TYPE));
    TEST_ASSERT_LESS_OR_EQUAL(MAX_TYPE_BYTES, strlen(LIGHT_ACTUATOR_TYPE));
    TEST_ASSERT_LESS_OR_EQUAL(MAX_TYPE_BYTES, strlen(LED_ACTUATOR_TYPE));
    TEST_ASSERT_LESS_OR_EQUAL(MAX_TYPE_BYTES, strlen(VALVE_ACTUATOR_TYPE));
}

// BCS-AC-002/BCS-AC-021: an oversized name is rejected observably before the
// registration, without consuming a slot and without leaving a partial
// capability or adapter behind.
void test_oversized_name_is_rejected_before_registration()
{
    const std::string overName(MAX_NAME_BYTES + 1, 'x');

    app::SwitchConfig cfg;
    cfg.GPIO = 5;
    cfg.capability_name = overName.c_str();

    auto *cap = builder->addSwitch(cfg);

    TEST_ASSERT_NULL(cap);                          // observable failure
    TEST_ASSERT_EQUAL(0, builder->count());         // no slot consumed
    TEST_ASSERT_EQUAL(0, hwFactory.outputsCreated); // no adapter constructed
    TEST_ASSERT_NULL(capSlots[0]);
    TEST_ASSERT_NULL(adapterSlots[0]);

    // A valid identity still registers normally afterwards.
    cfg.capability_name = "dev1_switch_ok";
    auto *ok = builder->addSwitch(cfg);
    TEST_ASSERT_NOT_NULL(ok);
    TEST_ASSERT_EQUAL(1, builder->count());
}

// BCS-AC-002: an automatically generated name that would exceed the limit is
// rejected too — the check applies to the definitive name.
void test_oversized_generated_name_is_rejected()
{
    deviceIdentity.deviceId = std::string(MAX_NAME_BYTES, 'd');
    rebuildBuilder();

    app::SwitchConfig cfg;
    cfg.GPIO = 5;

    auto *cap = builder->addSwitch(cfg);
    TEST_ASSERT_NULL(cap);
    TEST_ASSERT_EQUAL(0, builder->count());
    TEST_ASSERT_EQUAL(0, hwFactory.outputsCreated);

    deviceIdentity.deviceId = "dev1";
}

// ---------------------------------------------------------------------------
// CAP-AC-001/CAP-AC-002 — the selected capacity accepts exactly its configured
// number of capability/adapter pairs and rejects the next one atomically.
// ---------------------------------------------------------------------------
void test_configured_capacity_is_accepted_and_the_next_is_refused()
{
    char name[32];
    for (size_t i = 0; i < config::kMaxCapabilities; ++i)
    {
        snprintf(name, sizeof(name), "dev1_switch_%u", static_cast<unsigned>(i));
        app::SwitchConfig cfg;
        cfg.GPIO = static_cast<uint8_t>(5 + i);
        cfg.capability_name = name;
        TEST_ASSERT_NOT_NULL(builder->addSwitch(cfg));
    }
    TEST_ASSERT_EQUAL(config::kMaxCapabilities, builder->count());

    const size_t arenaBefore = builder->remainingArenaBytes();
    const int adaptersBefore = hwFactory.outputsCreated;
    app::SwitchConfig overflow;
    overflow.GPIO = 31;
    overflow.capability_name = "dev1_switch_overflow";
    TEST_ASSERT_NULL(builder->addSwitch(overflow));
    TEST_ASSERT_EQUAL(config::kMaxCapabilities, builder->count());
    TEST_ASSERT_EQUAL(adaptersBefore, hwFactory.outputsCreated);
    TEST_ASSERT_EQUAL(arenaBefore, builder->remainingArenaBytes());
}

// CAP-AC-003 — closing registration at setup is one-way and a late attempt
// cannot consume a capability, adapter or arena byte.
void test_closed_registration_rejects_late_capability_atomically()
{
    builder->closeRegistration();
    const size_t arenaBefore = builder->remainingArenaBytes();
    app::SwitchConfig late{5, true, "late_switch"};
    TEST_ASSERT_NULL(builder->addSwitch(late));
    TEST_ASSERT_EQUAL(0, builder->count());
    TEST_ASSERT_EQUAL(0, hwFactory.outputsCreated);
    TEST_ASSERT_EQUAL(arenaBefore, builder->remainingArenaBytes());
}

// CAP-AC-004 — adapter and arena exhaustion are distinct from slot exhaustion
// and leave the builder at its previous checkpoint.
void test_adapter_and_arena_exhaustion_are_atomic()
{
    core::ICapability *localCaps[1]{};
    void (*localCapDtors[1])(void *){};
    void *localAdapters[1]{};
    void (*localAdapterDtors[1])(void *){};
    uint8_t localArena[config::kCapabilityArenaBytes]{};

    app::CapabilitiesBuilder noAdapters(
        hwFactory, eventSink, localCaps, localCapDtors, 1,
        localAdapters, localAdapterDtors, 0,
        localArena, sizeof(localArena), deviceIdentity);
    const size_t adapterArenaBefore = noAdapters.remainingArenaBytes();
    TEST_ASSERT_NULL(noAdapters.addFan(app::FanConfig{5, true, "no_adapter"}));
    TEST_ASSERT_EQUAL(0, noAdapters.count());
    TEST_ASSERT_EQUAL(adapterArenaBefore, noAdapters.remainingArenaBytes());

    uint8_t tinyArena[sizeof(test::mocks::NoopCommandAdapter)]{};
    app::CapabilitiesBuilder noCapabilityArena(
        hwFactory, eventSink, localCaps, localCapDtors, 1,
        localAdapters, localAdapterDtors, 1,
        tinyArena, sizeof(tinyArena), deviceIdentity);
    const size_t capabilityArenaBefore = noCapabilityArena.remainingArenaBytes();
    TEST_ASSERT_NULL(noCapabilityArena.addFan(app::FanConfig{6, true, "no_capability_arena"}));
    TEST_ASSERT_EQUAL(0, noCapabilityArena.count());
    TEST_ASSERT_EQUAL(capabilityArenaBefore, noCapabilityArena.remainingArenaBytes());
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_identity_is_readable_but_not_assignable);
    RUN_TEST(test_deprecated_rename_preserves_the_registered_identity);
    RUN_TEST(test_omitted_name_uses_automatic_generation);
    RUN_TEST(test_identities_at_the_public_limits_are_accepted);
    RUN_TEST(test_oversized_name_is_rejected_before_registration);
    RUN_TEST(test_oversized_generated_name_is_rejected);
    RUN_TEST(test_configured_capacity_is_accepted_and_the_next_is_refused);
    RUN_TEST(test_closed_registration_rejects_late_capability_atomically);
    RUN_TEST(test_adapter_and_arena_exhaustion_are_atomic);
    UNITY_END();
}

void loop() {}
