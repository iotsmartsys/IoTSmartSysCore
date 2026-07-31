#include <Arduino.h>
#include <unity.h>

#include "Contracts/Capabilities/SwitchCapability.h"
#include "Contracts/Capabilities/ValveCapability.h"
#include "Contracts/Capabilities/LEDCapability.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.h"
#include "mocks/MockBinaryHardwareAdapter.h"
#include "mocks/MockEventSink.h"
#include "mocks/FakeBinaryCapabilityStateProvider.h"
#include "mocks/MockTimeProvider.h"

using namespace iotsmartsys;

static test::mocks::FakeBinaryCapabilityStateProvider fakeStorage;
static test::mocks::MockEventSink eventSink;
static test::mocks::MockTimeProvider mockTime;

void setUp(void)
{
    fakeStorage.records.clear();
    fakeStorage.saveCalls = 0;
    fakeStorage.loadSnapshotCalls = 0;
    fakeStorage.failNextSave = false;
    eventSink.clear();
    core::ServiceProvider::instance().setBinaryCapabilityStateProvider(&fakeStorage);
    mockTime.currentMs = 0;
    core::Time::setProvider(&mockTime);
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

// BCS-003/BCS-004/BCS-009/BCS-010/5.3: storage only knows the semantic
// off/on state; restoration must reuse the same interpreted command path as
// a normal command, so ValveCapability's open/closed vocabulary is produced
// by the interpreter, never sent directly to the adapter.
void test_valve_vocabulary_conversion_on_restore_open()
{
    fakeStorage.seed("dev1_valve", "Valve Actuator", true); // true == "open"
    test::mocks::MockBinaryHardwareAdapter adapter("off"); // adapter-native vocabulary
    core::ValveCapability cap("dev1_valve", adapter, &eventSink);
    core::ValveHardwareCommandInterpreter interpreter;
    cap.setCommandInterpreter(&interpreter);

    cap.setup();

    TEST_ASSERT_TRUE(cap.isOpen());
    // The adapter (fidelity double, 8.2) only ever sees "on"/"off"/"toggle".
    TEST_ASSERT_EQUAL_STRING("on", adapter.getStateValue().c_str());
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls); // restore never persists (BCS-011)
}

void test_valve_vocabulary_conversion_on_restore_closed()
{
    fakeStorage.seed("dev1_valve", "Valve Actuator", false); // false == "closed"
    test::mocks::MockBinaryHardwareAdapter adapter("on");
    core::ValveCapability cap("dev1_valve", adapter, &eventSink);
    core::ValveHardwareCommandInterpreter interpreter;
    cap.setCommandInterpreter(&interpreter);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOpen());
    TEST_ASSERT_EQUAL_STRING("off", adapter.getStateValue().c_str());
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls);
}

// BCS-016/BCS-AC-015: LEDCapability::handle() overrides the base handle()
// but must not bypass sync/publish/persist for non-blink commands.
void test_led_handle_outside_blink_persists_confirmed_command()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::LEDCapability led("dev1_led", adapter, &eventSink);
    led.setup();
    TEST_ASSERT_EQUAL(0, fakeStorage.saveCalls);

    led.executeCommand("on");
    led.handle(); // must sync/publish/persist even though blink is not active

    TEST_ASSERT_TRUE(led.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);
    TEST_ASSERT_TRUE(fakeStorage.lastSavedIsOn);
}

// BCS-016/BCS-AC-015: each confirmed blink alternation is a transition and
// must produce exactly one persist.
void test_led_blink_persists_each_alternation()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::LEDCapability led("dev1_led", adapter, &eventSink);
    led.setup();

    led.blink(10);
    mockTime.advance(10);
    led.handle(); // first alternation: off -> on
    TEST_ASSERT_TRUE(led.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);

    led.handle(); // no interval elapsed: no new alternation
    TEST_ASSERT_EQUAL(1, fakeStorage.saveCalls);

    mockTime.advance(10);
    led.handle(); // second alternation: on -> off
    TEST_ASSERT_FALSE(led.isOn());
    TEST_ASSERT_EQUAL(2, fakeStorage.saveCalls);
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
    RUN_TEST(test_valve_vocabulary_conversion_on_restore_open);
    RUN_TEST(test_valve_vocabulary_conversion_on_restore_closed);
    RUN_TEST(test_led_handle_outside_blink_persists_confirmed_command);
    RUN_TEST(test_led_blink_persists_each_alternation);
    UNITY_END();
}

void loop() {}
