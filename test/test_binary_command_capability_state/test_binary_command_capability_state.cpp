#include <Arduino.h>
#include <unity.h>

#include "Contracts/Capabilities/SwitchCapability.h"
#include "Contracts/Capabilities/SwitchPlugCapability.h"
#include "Contracts/Capabilities/LightCapability.h"
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
    fakeStorage.clear();
    eventSink.clear();
    core::ServiceProvider::instance().setBinaryCapabilityStateProvider(&fakeStorage);
    mockTime.currentMs = 0;
    core::Time::setProvider(&mockTime);
    fakeStorage.activateWriter();
}

void tearDown(void) {}

// ---------------------------------------------------------------------------
// BCS-AC-011 — absence of a record preserves the vigent default and writes
// nothing just for booting at the default.
// ---------------------------------------------------------------------------
void test_setup_without_record_uses_default()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);
    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(0, fakeStorage.writes);
    TEST_ASSERT_EQUAL(0, fakeStorage.commits);
}

// ---------------------------------------------------------------------------
// BCS-AC-005 — adapter setup runs first, the restored value is applied through
// the interpreted path, and only a confirming read-back promotes it to the
// logical state. Covers switch, switch plug, light and LED.
// ---------------------------------------------------------------------------
void test_setup_restores_valid_record_for_every_off_on_type()
{
    {
        fakeStorage.seed("dev1_switch", SWITCH_TYPE, true);
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
        cap.setup();
        TEST_ASSERT_TRUE(cap.isOn());
        TEST_ASSERT_EQUAL(1, adapter.setupCalls);
        TEST_ASSERT_EQUAL(1, adapter.applyCommandCalls);
    }
    {
        fakeStorage.seed("dev1_plug", SWITCH_PLUG_TYPE, true);
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::SwitchPlugCapability cap("dev1_plug", adapter, &eventSink);
        cap.setup();
        TEST_ASSERT_TRUE(cap.isOn());
    }
    {
        fakeStorage.seed("dev1_light", LIGHT_ACTUATOR_TYPE, true);
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::LightCapability cap("dev1_light", adapter, &eventSink);
        cap.setup();
        TEST_ASSERT_TRUE(cap.isOn());
    }
    {
        fakeStorage.seed("dev1_led", LED_ACTUATOR_TYPE, false);
        test::mocks::MockBinaryHardwareAdapter adapter("on");
        core::LEDCapability cap("dev1_led", adapter, &eventSink);
        cap.setup();
        TEST_ASSERT_FALSE(cap.isOn());
    }

    // BCS-011: restoring never persists.
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);
}

// ---------------------------------------------------------------------------
// BCS-AC-012 — a rejected command and a non-confirming read-back both keep the
// default; the requested value is neither announced nor persisted.
// ---------------------------------------------------------------------------
void test_setup_restore_rejected_by_adapter_keeps_default()
{
    fakeStorage.seed("dev1_switch", SWITCH_TYPE, true);
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    adapter.acceptCommands = false;
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);
}

void test_setup_restore_unconfirmed_keeps_default()
{
    fakeStorage.seed("dev1_switch", SWITCH_TYPE, true);
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    adapter.confirmMismatch = true;
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);
}

// ---------------------------------------------------------------------------
// BCS-AC-013 — an isolated stable change signals the writer once and ends in a
// commit; repeating the same value produces no further signal, write or commit.
// ---------------------------------------------------------------------------
void test_transition_signals_once_and_repeats_do_not()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);

    cap.turnOn();
    cap.handle(); // confirms the transition via syncFromHardware

    TEST_ASSERT_TRUE(cap.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    TEST_ASSERT_EQUAL_STRING("dev1_switch", fakeStorage.lastRequestedName.c_str());
    TEST_ASSERT_EQUAL_STRING(SWITCH_TYPE, fakeStorage.lastRequestedType.c_str());
    TEST_ASSERT_TRUE(fakeStorage.lastRequestedIsOn);

    // The requesting context performed no NVS work at all.
    TEST_ASSERT_EQUAL(0, fakeStorage.writes);
    TEST_ASSERT_EQUAL(0, fakeStorage.commits);
    TEST_ASSERT_EQUAL(1, fakeStorage.pendingCount());

    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.writes);
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);
    TEST_ASSERT_EQUAL(0, fakeStorage.pendingCount());

    cap.handle(); // no new hardware transition
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);

    cap.turnOn(); // requesting the current logical value again
    cap.handle();
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);

    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.writes);
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);
}

// ---------------------------------------------------------------------------
// BCS-AC-013 — a burst before the writer runs keeps at most one pending entry
// per identity and consolidates it into the most recent value.
// ---------------------------------------------------------------------------
void test_burst_consolidates_into_single_pending_entry()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    cap.turnOn();
    cap.handle();
    cap.turnOff();
    cap.handle();
    cap.turnOn();
    cap.handle();

    TEST_ASSERT_EQUAL(3, fakeStorage.requestCalls);
    TEST_ASSERT_EQUAL(1, fakeStorage.pendingCount()); // one entry for the identity
    TEST_ASSERT_EQUAL(0, fakeStorage.writes);

    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.writes);
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);

    // Simulated reboot: the committed value is the most recent stable one.
    bool restored = false;
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_switch", SWITCH_TYPE, restored));
    TEST_ASSERT_TRUE(restored);
}

// ---------------------------------------------------------------------------
// BCS-AC-003/BCS-AC-014 — toggle persists only the final confirmed state, and
// every authorised origin follows the same protocol.
// ---------------------------------------------------------------------------
void test_toggle_persists_only_final_confirmed_value()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    cap.toggle();
    cap.handle();
    fakeStorage.releaseWriter();

    TEST_ASSERT_TRUE(cap.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    TEST_ASSERT_TRUE(fakeStorage.lastRequestedIsOn);
    // The transitory command text never reaches the storage: only the boolean
    // semantic state does.
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);

    cap.toggle();
    cap.handle();
    fakeStorage.releaseWriter();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_EQUAL(2, fakeStorage.requestCalls);
    TEST_ASSERT_FALSE(fakeStorage.lastRequestedIsOn);
}

void test_every_authorised_origin_follows_the_same_protocol()
{
    struct Origin
    {
        const char *name;
        void (*apply)(core::SwitchCapability &, test::mocks::MockBinaryHardwareAdapter &);
    };

    const Origin origins[] = {
        {"turnOn", [](core::SwitchCapability &c, test::mocks::MockBinaryHardwareAdapter &) { c.turnOn(); }},
        {"turnOff", [](core::SwitchCapability &c, test::mocks::MockBinaryHardwareAdapter &) { c.turnOff(); }},
        {"power", [](core::SwitchCapability &c, test::mocks::MockBinaryHardwareAdapter &) { c.power("on"); }},
        {"toggle", [](core::SwitchCapability &c, test::mocks::MockBinaryHardwareAdapter &) { c.toggle(); }},
        // External change observed on the adapter, with no command issued.
        {"adapter", [](core::SwitchCapability &, test::mocks::MockBinaryHardwareAdapter &a) { a.state = "on"; }},
    };

    for (const auto &origin : origins)
    {
        fakeStorage.clear();
        fakeStorage.activateWriter();
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
        cap.setup();

        origin.apply(cap, adapter);
        cap.handle();

        if (cap.isOn())
        {
            // A confirmed stable change signals the writer exactly once and
            // ends in a commit.
            TEST_ASSERT_EQUAL_MESSAGE(1, fakeStorage.requestCalls, origin.name);
            fakeStorage.releaseWriter();
            TEST_ASSERT_EQUAL_MESSAGE(1, fakeStorage.commits, origin.name);
        }
        else
        {
            // No change: no signal, no write, no commit.
            TEST_ASSERT_EQUAL_MESSAGE(0, fakeStorage.requestCalls, origin.name);
            fakeStorage.releaseWriter();
            TEST_ASSERT_EQUAL_MESSAGE(0, fakeStorage.commits, origin.name);
        }
    }
}

// ---------------------------------------------------------------------------
// BCS-AC-018 — restoring or changing one capability never touches another.
// ---------------------------------------------------------------------------
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
    fakeStorage.releaseWriter();

    TEST_ASSERT_TRUE(cap1.isOn());
    TEST_ASSERT_FALSE(cap2.isOn());
    TEST_ASSERT_EQUAL_STRING("off", adapter2.state.c_str());

    bool dev2On = true;
    TEST_ASSERT_FALSE(fakeStorage.tryGet("dev2_switch", SWITCH_TYPE, dev2On));

    bool dev1On = false;
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_switch", SWITCH_TYPE, dev1On));
    TEST_ASSERT_TRUE(dev1On);
}

// ---------------------------------------------------------------------------
// BCS-AC-019 — a different name or a different type is a different identity and
// never reuses the previous record.
// ---------------------------------------------------------------------------
void test_changed_identity_does_not_reuse_previous_record()
{
    fakeStorage.seed("dev1_switch", SWITCH_TYPE, true);

    {
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::SwitchCapability renamed("dev1_switch_renamed", adapter, &eventSink);
        renamed.setup();
        TEST_ASSERT_FALSE(renamed.isOn()); // default preserved
    }
    {
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::SwitchPlugCapability retyped("dev1_switch", adapter, &eventSink);
        retyped.setup();
        TEST_ASSERT_FALSE(retyped.isOn()); // same name, different type
    }
    {
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        core::SwitchCapability original("dev1_switch", adapter, &eventSink);
        original.setup();
        TEST_ASSERT_TRUE(original.isOn()); // the original identity still resolves
    }
}

// ---------------------------------------------------------------------------
// BCS-AC-017 — a write or commit failure never reverts the confirmed state; the
// snapshot that survives a reboot is the last *successful* commit.
// ---------------------------------------------------------------------------
void test_write_failure_keeps_confirmed_state_and_last_successful_commit()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    cap.turnOff();
    cap.handle();
    cap.turnOn();
    cap.handle();
    fakeStorage.failNextWrite = true;
    fakeStorage.releaseWriter();

    TEST_ASSERT_TRUE(cap.isOn()); // hardware/logical state unaffected
    TEST_ASSERT_EQUAL_STRING("on", adapter.state.c_str());
    TEST_ASSERT_EQUAL(1, fakeStorage.failures);
    bool stored = true;
    TEST_ASSERT_FALSE(fakeStorage.tryGet("dev1_switch", SWITCH_TYPE, stored)); // never landed

    // A later confirmed stable transition retries; no continuous retry loop.
    cap.turnOff();
    cap.handle();
    fakeStorage.releaseWriter();

    TEST_ASSERT_FALSE(cap.isOn());
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_switch", SWITCH_TYPE, stored));
    TEST_ASSERT_FALSE(stored);
}

void test_commit_failure_is_not_promoted_to_success()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    cap.turnOn();
    cap.handle();
    fakeStorage.failNextCommit = true;
    fakeStorage.releaseWriter();

    TEST_ASSERT_TRUE(cap.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.writes);
    TEST_ASSERT_EQUAL(0, fakeStorage.commits);
    TEST_ASSERT_EQUAL(1, fakeStorage.failures);
    bool stored = true;
    TEST_ASSERT_FALSE(fakeStorage.tryGet("dev1_switch", SWITCH_TYPE, stored));
}

// ---------------------------------------------------------------------------
// BCS-AC-028 — a writer that could not be created stays observably unavailable
// and never causes a synchronous fallback; the cooperative cycle continues.
// ---------------------------------------------------------------------------
void test_writer_creation_failure_has_no_synchronous_fallback()
{
    fakeStorage.clear();
    fakeStorage.failWriterCreation = true;
    TEST_ASSERT_NOT_EQUAL(static_cast<int>(core::common::StateResult::Ok),
                          static_cast<int>(fakeStorage.activateWriter()));
    TEST_ASSERT_FALSE(fakeStorage.writerStatus().available);

    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::SwitchCapability cap("dev1_switch", adapter, &eventSink);
    cap.setup();

    cap.turnOn();
    cap.handle();
    cap.handle(); // the cooperative cycle keeps running

    TEST_ASSERT_TRUE(cap.isOn()); // hardware/logical state preserved
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    TEST_ASSERT_EQUAL(0, fakeStorage.writes);
    TEST_ASSERT_EQUAL(0, fakeStorage.commits);
}

// ---------------------------------------------------------------------------
// BCS-AC-004 — the valve's open/closed vocabulary is produced by the
// interpreter in restore, sync and every fallback; off/on never crosses into
// the valve's logical state, and the adapter never receives open/closed.
// ---------------------------------------------------------------------------
void test_valve_vocabulary_conversion_on_restore_open()
{
    fakeStorage.seed("dev1_valve", VALVE_ACTUATOR_TYPE, true); // true == "open"
    test::mocks::MockBinaryHardwareAdapter adapter("off");     // adapter-native vocabulary
    core::ValveCapability cap("dev1_valve", adapter, &eventSink);
    core::ValveHardwareCommandInterpreter interpreter;
    cap.setCommandInterpreter(&interpreter);

    cap.setup();

    TEST_ASSERT_TRUE(cap.isOpen());
    TEST_ASSERT_EQUAL_STRING(VALVE_STATE_OPEN, cap.value.c_str());
    // The adapter (fidelity double, 8.2) only ever sees on/off/toggle.
    TEST_ASSERT_EQUAL_STRING("on", adapter.getStateValue().c_str());
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);
}

void test_valve_vocabulary_conversion_on_restore_closed()
{
    fakeStorage.seed("dev1_valve", VALVE_ACTUATOR_TYPE, false); // false == "closed"
    test::mocks::MockBinaryHardwareAdapter adapter("on");
    core::ValveCapability cap("dev1_valve", adapter, &eventSink);
    core::ValveHardwareCommandInterpreter interpreter;
    cap.setCommandInterpreter(&interpreter);

    cap.setup();

    TEST_ASSERT_FALSE(cap.isOpen());
    TEST_ASSERT_EQUAL_STRING(VALVE_STATE_CLOSED, cap.value.c_str());
    TEST_ASSERT_EQUAL_STRING("off", adapter.getStateValue().c_str());
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);
}

// BCS-AC-004/BCS-028: the fallbacks — no record at all, a rejected restore and
// an unconfirmed restore — must also go through the interpreter. Reading
// getStateValue() and promoting off/on would be a failure.
void test_valve_fallbacks_never_promote_adapter_vocabulary()
{
    {
        // No record: initial sync.
        test::mocks::MockBinaryHardwareAdapter adapter("on");
        core::ValveCapability cap("dev1_valve", adapter, &eventSink);
        core::ValveHardwareCommandInterpreter interpreter;
        cap.setCommandInterpreter(&interpreter);
        cap.setup();
        TEST_ASSERT_EQUAL_STRING(VALVE_STATE_OPEN, cap.value.c_str());
    }
    {
        // Restore rejected by the adapter.
        fakeStorage.clear();
        fakeStorage.activateWriter();
        fakeStorage.seed("dev1_valve", VALVE_ACTUATOR_TYPE, true);
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        adapter.acceptCommands = false;
        core::ValveCapability cap("dev1_valve", adapter, &eventSink);
        core::ValveHardwareCommandInterpreter interpreter;
        cap.setCommandInterpreter(&interpreter);
        cap.setup();
        TEST_ASSERT_EQUAL_STRING(VALVE_STATE_CLOSED, cap.value.c_str());
    }
    {
        // Applied but not confirmed by the read-back.
        fakeStorage.clear();
        fakeStorage.activateWriter();
        fakeStorage.seed("dev1_valve", VALVE_ACTUATOR_TYPE, true);
        test::mocks::MockBinaryHardwareAdapter adapter("off");
        adapter.confirmMismatch = true;
        core::ValveCapability cap("dev1_valve", adapter, &eventSink);
        core::ValveHardwareCommandInterpreter interpreter;
        cap.setCommandInterpreter(&interpreter);
        cap.setup();
        TEST_ASSERT_EQUAL_STRING(VALVE_STATE_CLOSED, cap.value.c_str());
    }
}

// BCS-AC-004: a confirmed valve transition persists the semantic state, never
// the concrete open/closed text, and reaches the adapter as on/off.
void test_valve_transition_persists_semantic_state()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::ValveCapability cap("dev1_valve", adapter, &eventSink);
    core::ValveHardwareCommandInterpreter interpreter;
    cap.setCommandInterpreter(&interpreter);
    cap.setup();

    cap.turnOpen();
    cap.handle();
    fakeStorage.releaseWriter();

    TEST_ASSERT_TRUE(cap.isOpen());
    TEST_ASSERT_EQUAL_STRING("on", adapter.state.c_str());
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    TEST_ASSERT_TRUE(fakeStorage.lastRequestedIsOn);
    TEST_ASSERT_EQUAL_STRING(VALVE_ACTUATOR_TYPE, fakeStorage.lastRequestedType.c_str());
}

// ---------------------------------------------------------------------------
// BCS-AC-015 — LEDCapability::handle() overrides handle() but never bypasses
// the common protocol; blink alternations stay transitory.
// ---------------------------------------------------------------------------
void test_led_handle_outside_blink_persists_confirmed_command()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::LEDCapability led("dev1_led", adapter, &eventSink);
    led.setup();
    TEST_ASSERT_EQUAL(0, fakeStorage.requestCalls);

    led.executeCommand("on");
    led.handle(); // must sync/publish/persist even though blink is not active
    fakeStorage.releaseWriter();

    TEST_ASSERT_TRUE(led.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);
    TEST_ASSERT_TRUE(fakeStorage.lastRequestedIsOn);
}

void test_led_blink_alternations_are_transitory_and_never_persist()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::LEDCapability led("dev1_led", adapter, &eventSink);
    led.setup();

    // A stable state is persisted first.
    led.executeCommand("on");
    led.handle();
    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);

    led.blink(10);
    for (int i = 0; i < 5; ++i)
    {
        mockTime.advance(10);
        led.handle();
        // Each alternation is applied, confirmed and published...
        TEST_ASSERT_EQUAL_STRING(led.isOn() ? "on" : "off", adapter.state.c_str());
    }

    // ...but none of them signals the writer, and the previously persisted
    // stable state is untouched.
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);
    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.writes);
    TEST_ASSERT_EQUAL(1, fakeStorage.commits);
    bool persisted = false;
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_led", LED_ACTUATOR_TYPE, persisted));
    TEST_ASSERT_TRUE(persisted);
}

void test_led_leaving_blink_requests_the_new_stable_state_once()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::LEDCapability led("dev1_led", adapter, &eventSink);
    led.setup();

    led.executeCommand("on");
    led.handle();
    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);

    // Blink an odd number of alternations, so the mode ends on the opposite
    // state of the last stable one.
    led.blink(10);
    mockTime.advance(10);
    led.handle();
    TEST_ASSERT_FALSE(led.isOn());
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);

    led.blink(0); // leaving blink turns the current value into a stable one

    TEST_ASSERT_EQUAL(2, fakeStorage.requestCalls);
    TEST_ASSERT_FALSE(fakeStorage.lastRequestedIsOn);

    led.blink(0); // idempotent: already stopped, nothing new is requested
    TEST_ASSERT_EQUAL(2, fakeStorage.requestCalls);

    fakeStorage.releaseWriter();
    bool persisted = true;
    TEST_ASSERT_TRUE(fakeStorage.tryGet("dev1_led", LED_ACTUATOR_TYPE, persisted));
    TEST_ASSERT_FALSE(persisted);
}

void test_led_leaving_blink_on_unchanged_state_requests_nothing()
{
    test::mocks::MockBinaryHardwareAdapter adapter("off");
    core::LEDCapability led("dev1_led", adapter, &eventSink);
    led.setup();

    led.executeCommand("on");
    led.handle();
    fakeStorage.releaseWriter();
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls);

    // An even number of alternations returns to the last stable state.
    led.blink(10);
    mockTime.advance(10);
    led.handle();
    mockTime.advance(10);
    led.handle();
    TEST_ASSERT_TRUE(led.isOn());

    led.blink(0);
    TEST_ASSERT_EQUAL(1, fakeStorage.requestCalls); // unchanged: nothing requested
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_setup_without_record_uses_default);
    RUN_TEST(test_setup_restores_valid_record_for_every_off_on_type);
    RUN_TEST(test_setup_restore_rejected_by_adapter_keeps_default);
    RUN_TEST(test_setup_restore_unconfirmed_keeps_default);
    RUN_TEST(test_transition_signals_once_and_repeats_do_not);
    RUN_TEST(test_burst_consolidates_into_single_pending_entry);
    RUN_TEST(test_toggle_persists_only_final_confirmed_value);
    RUN_TEST(test_every_authorised_origin_follows_the_same_protocol);
    RUN_TEST(test_identity_isolation_between_capabilities);
    RUN_TEST(test_changed_identity_does_not_reuse_previous_record);
    RUN_TEST(test_write_failure_keeps_confirmed_state_and_last_successful_commit);
    RUN_TEST(test_commit_failure_is_not_promoted_to_success);
    RUN_TEST(test_writer_creation_failure_has_no_synchronous_fallback);
    RUN_TEST(test_valve_vocabulary_conversion_on_restore_open);
    RUN_TEST(test_valve_vocabulary_conversion_on_restore_closed);
    RUN_TEST(test_valve_fallbacks_never_promote_adapter_vocabulary);
    RUN_TEST(test_valve_transition_persists_semantic_state);
    RUN_TEST(test_led_handle_outside_blink_persists_confirmed_command);
    RUN_TEST(test_led_blink_alternations_are_transitory_and_never_persist);
    RUN_TEST(test_led_leaving_blink_requests_the_new_stable_state_once);
    RUN_TEST(test_led_leaving_blink_on_unchanged_state_requests_nothing);
    UNITY_END();
}

void loop() {}
