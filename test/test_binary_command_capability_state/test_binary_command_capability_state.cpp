#include <Arduino.h>
#include <unity.h>

#include "Contracts/Capabilities/SwitchCapability.h"
#include "Contracts/Capabilities/ValveCapability.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "mocks/MockBinaryHardwareAdapter.h"
#include "mocks/MockEventSink.h"
#include "mocks/FakeBinaryCapabilityStateProvider.h"

using namespace iotsmartsys;

static test::mocks::FakeBinaryCapabilityStateProvider fakeStorage;
static test::mocks::MockEventSink eventSink;

void setUp(void)
{
    fakeStorage.records.clear();
    fakeStorage.saveCalls = 0;
    fakeStorage.loadSnapshotCalls = 0;
    fakeStorage.failNextSave = false;
    eventSink.clear();
    core::ServiceProvider::instance().setBinaryCapabilityStateProvider(&fakeStorage);
}

void tearDown(void) {}

// BCS-011: no persisted record -> the vigent default is preserved and nothing
// is written.
void test_setup_without_record_uses_default()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls);
}

// BCS-009/BCS-010: setup applies the adapter first, then a valid record is
// applied and only becomes the logical state after read-back confirmation.
void test_setup_restores_valid_record()
{
    fakeStorage.seed("dev1_switch", "Switch", true);
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_TRUE(cap.isOn());
    TEST_ASSERT_EQUAL(1, adapter.setupCalls);
    TEST_ASSERT_EQUAL(1, adapter.applyCommandCalls);
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls); // restore never persists (BCS-011)
}

// BCS-011/BCS-012: adapter rejection during restore keeps the default flow.
void test_setup_restore_rejected_by_adapter_keeps_default()
{
    fakeStorage.seed("dev1_switch", "Switch", true);
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    adapter.acceptCommands = false;
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls);
}

// BCS-010: acceptance without a confirming read-back must not be announced
// as applied.
void test_setup_restore_unconfirmed_keeps_default()
{
    fakeStorage.seed("dev1_switch", "Switch", true);
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    adapter.confirmMismatch = true;
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls);
}

// BCS-013/BCS-014: every confirmed transition persists exactly once; a
// request that repeats the already-confirmed value never re-persists.
void test_transition_persists_once_and_repeats_do_not()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls);

    cap.turnOn();
    cap.handle(); // confirms the transition via syncFromHardware

    TEST_ASSERT_TRUE(cap.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);
    TEST_ASSERT_EQUAL_STRING("dev1_switch", fakeStorage.lastSavedName.c_str());
    TEST_ASSERT_EQUAL_STRING("Switch", fakeStorage.lastSavedType.c_str());
    TEST_ASSERT_TRUE(fakeStorage.lastSavedIsOn);

    cap.handle(); // no new hardware transition
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);

    cap.turnOn(); // requesting the current logical value again
    cap.handle();
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);
}

// BCS-015: toggle must never persist the transitory command text, only the
// final confirmed boolean state.
void test_toggle_persists_only_final_confirmed_value()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    cap.toggle();
    cap.handle();

    TEST_ASSERT_TRUE(cap.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);
    TEST_ASSERT_TRUE(fakeStorage.lastSavedIsOn);
}

// BCS-002/BCS-019: identity is (capability_name, type); two capabilities
// never cross-apply or cross-persist each other's state.
void test_identity_isolation_between_capabilities()
{
    test::mocks::MockBinaryHardwareAdapter adapter1("off");
    test::mocks::MockBinaryHardwareAdapter adapter2("off");
    core::SwitchCapability cap1("dev1_switch", adapter1, &eventSink);
    core::SwitchCapability cap2("dev2_switch", adapter2, &eventSink);
    cap1.setup();
    cap2.setup();

    cap1.turnOn();
    cap1.handle();

    TEST_ASSERT_TRUE(cap1.isOn());
    TEST_ASSERT_FALSE(cap2.isOn());

    bool dev2On = true;
    TEST_ASSERT_FALSE(fakeStorage.tryGet("dev2_switch", "Switch", dev2On));

    bool dev1On = false;
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_switch", "Switch", dev1On));
    TEST_ASSERT_TRUE(dev1On);
}

// BCS-017/BCS-018: a persistence failure never reverts the already-applied
// hardware/logical state; the retained record is the last *successful*
// commit, retried only on the next real transition.
void test_persist_failure_keeps_hardware_state_and_last_successful_record()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    fakeStorage.failNextSave = true;
    cap.turnOn();
    cap.handle();

    TEST_ASSERT_TRUE(cap.isOn()); // hardware/logical state unaffected by the failure
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);
    bool stored = false;
    TEST_ASSERT_FALSE(fakeStorage.tryGet("dev1_switch", "Switch", stored)); // failed write never landed

    cap.turnOff();
    cap.handle();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(2, fakeStorage.saveCalls);
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_switch", "Switch", stored));
    TEST_ASSERT_FALSE(stored); // last successful commit reflects the latest confirmed state
}

// BCS-003/BCS-004: storage only knows the semantic off/on state; ValveCapability
// converts it to its own open/closed vocabulary on restore.
void test_valve_vocabulary_conversion_on_restore()
{
    fakeStorage.seed("dev1_valve", "Valve Actuator", true); // true == "open"
    test::mocks::MockBinaryHardwareAdapter adapter("closed");
    core::ValveCapability cap("dev1_valve", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_TRUE(cap.isOpen());
    TEST_ASSERT_EQUAL_STRING("open", adapter.getStateValue().c_str());
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_setup_without_record_uses_default);
    RUN_TEST(test_setup_restores_valid_record);
    RUN_TEST(test_setup_restore_rejected_by_adapter_keeps_default);
    RUN_TEST(test_setup_restore_unconfirmed_keeps_default);
    RUN_TEST(test_transition_persists_once_and_repeats_do_not);
    RUN_TEST(test_toggle_persists_only_final_confirmed_value);
    RUN_TEST(test_identity_isolation_between_capabilities);
    RUN_TEST(test_persist_failure_keeps_hardware_state_and_last_successful_record);
    RUN_TEST(test_valve_vocabulary_conversion_on_restore);
    UNITY_END();
}

void loop() {}
