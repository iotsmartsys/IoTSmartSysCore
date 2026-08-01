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

static core::ICapability *capSlots[8];
static void (*capDtors[8])(void *);
static void *adapterSlots[8];
static void (*adapterDtors[8])(void *);
static uint8_t arena[4096];

static app::CapabilitiesBuilder *builder = nullptr;
static uint8_t builderStorage[sizeof(app::CapabilitiesBuilder)];

static void rebuildBuilder()
{
    if (builder)
    {
        builder->reset();
        builder->~CapabilitiesBuilder();
    }
    for (size_t i = 0; i < 8; ++i)
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
        capSlots, capDtors, 8,
        adapterSlots, adapterDtors, 8,
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
// BCS-AC-021 — the eight-capability limit and the configuration order before
// SmartSysApp::setup() are preserved.
// ---------------------------------------------------------------------------
void test_eight_capabilities_are_accepted_and_the_ninth_is_refused()
{
    char name[32];
    for (int i = 0; i < 8; ++i)
    {
        snprintf(name, sizeof(name), "dev1_switch_%d", i);
        app::SwitchConfig cfg;
        cfg.GPIO = static_cast<uint8_t>(5 + i);
        cfg.capability_name = name;
        TEST_ASSERT_NOT_NULL(builder->addSwitch(cfg));
    }
    TEST_ASSERT_EQUAL(8, builder->count());

    app::SwitchConfig ninth;
    ninth.GPIO = 20;
    ninth.capability_name = "dev1_switch_9th";
    TEST_ASSERT_NULL(builder->addSwitch(ninth));
    TEST_ASSERT_EQUAL(8, builder->count());
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
    RUN_TEST(test_eight_capabilities_are_accepted_and_the_ninth_is_refused);
    UNITY_END();
}

void loop() {}
