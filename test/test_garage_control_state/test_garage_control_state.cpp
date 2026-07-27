#include <Arduino.h>
#include <unity.h>

#include "Contracts/Capabilities/GarageControlCapability.h"
#include "Contracts/Logging/Log.h"
#include "Platform/Arduino/Logging/ArduinoSerialLogger.h"
#include "mocks/MockInputAdapter.h"
#include "mocks/MockCommandAdapter.h"
#include "mocks/MockEventSink.h"
#include "mocks/MockTimeProvider.h"

using namespace iotsmartsys;

static platform::arduino::ArduinoSerialLogger logger(Serial);
static test::mocks::MockTimeProvider timeProvider;

static test::mocks::MockCommandAdapter openAdapter;
static test::mocks::MockCommandAdapter closeAdapter;
static test::mocks::MockCommandAdapter stopUnlockAdapter;
static test::mocks::MockCommandAdapter lockAdapter;
static test::mocks::MockInputAdapter openSensor;
static test::mocks::MockInputAdapter closeSensor;
static test::mocks::MockEventSink eventSink;

static void reset_fixtures()
{
    openAdapter.powered = false;
    openAdapter.pulseCount = 0;
    closeAdapter.powered = false;
    closeAdapter.pulseCount = 0;
    stopUnlockAdapter.powered = false;
    stopUnlockAdapter.pulseCount = 0;
    lockAdapter.powered = false;
    lockAdapter.pulseCount = 0;
    openSensor.setState(HIGH);
    closeSensor.setState(HIGH);
    eventSink.events.clear();
    timeProvider.set(0);
}

static void tick(core::GarageControlCapability &cap, std::uint64_t deltaMs = 0)
{
    timeProvider.advance(deltaMs);
    cap.handle();
}

static void assert_state(core::GarageControlCapability &cap, const char *expected)
{
    auto state = cap.readState();
    TEST_ASSERT_EQUAL_STRING(expected, state.value.c_str());
}

static void assert_event_count(size_t expected)
{
    TEST_ASSERT_EQUAL(expected, eventSink.events.size());
}

static void assert_last_event(const char *expected)
{
    assert_event_count(eventSink.events.size());
    TEST_ASSERT_FALSE(eventSink.events.empty());
    TEST_ASSERT_EQUAL_STRING(expected, eventSink.events.back().value.c_str());
}

// ----------------------------------------------------------------------------
// 1. Default and separation between sensor debounce and relay pulse.
// ----------------------------------------------------------------------------
void test_defaults_and_debounce_separation()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 200, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink); // sensorDebounceTimeMs defaults to 50

    cap.setup();
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_UNKNOWN);

    // A command issues a relay pulse controlled by debounceTimeMs, not sensorDebounceTimeMs.
    cap.open();
    TEST_ASSERT_EQUAL(1, openAdapter.pulseCount);

    // The relay pulse length must not influence the sensor debounce timing.
    closeSensor.setState(LOW);
    tick(cap, 49);
    assert_state(cap, GARAGE_STATE_UNKNOWN); // still bouncing
    tick(cap, 1);
    assert_state(cap, GARAGE_STATE_CLOSED);
}

// ----------------------------------------------------------------------------
// 2. Initialization in the four sensor combinations.
// ----------------------------------------------------------------------------
void test_initialization_combinations()
{
    // Both inactive -> unknown
    reset_fixtures();
    {
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            &openSensor, &closeSensor, &eventSink, 50);
        cap.setup();
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_UNKNOWN);
    }

    // Open active only -> opened
    reset_fixtures();
    {
        openSensor.setState(LOW);
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            &openSensor, &closeSensor, &eventSink, 50);
        cap.setup();
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_OPENED);
    }

    // Close active only -> closed
    reset_fixtures();
    {
        closeSensor.setState(LOW);
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            &openSensor, &closeSensor, &eventSink, 50);
        cap.setup();
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_CLOSED);
    }

    // Both active -> unknown
    reset_fixtures();
    {
        openSensor.setState(LOW);
        closeSensor.setState(LOW);
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            &openSensor, &closeSensor, &eventSink, 50);
        cap.setup();
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_UNKNOWN);
    }
}

// ----------------------------------------------------------------------------
// 3. closed -> opening -> opened and symmetric close flow.
// ----------------------------------------------------------------------------
void test_commanded_open_and_symmetric_close()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();

    // Start closed.
    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    // Command open.
    cap.open();
    TEST_ASSERT_EQUAL(1, openAdapter.pulseCount);
    // While the close endpoint remains active, state stays closed.
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    // Release the close endpoint -> opening.
    closeSensor.setState(HIGH);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENING);

    // Reach the open endpoint -> opened.
    openSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENED);

    // Command close.
    cap.close();
    TEST_ASSERT_EQUAL(1, closeAdapter.pulseCount);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENED); // still active

    openSensor.setState(HIGH);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSING);

    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);
}

// ----------------------------------------------------------------------------
// 4. Failed start keeps the origin endpoint.
// ----------------------------------------------------------------------------
void test_failed_start_keeps_origin()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();

    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    cap.open();
    // The gate does not move; close endpoint stays active.
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);
    tick(cap, 200);
    assert_state(cap, GARAGE_STATE_CLOSED);
}

// ----------------------------------------------------------------------------
// 5. Return to the origin endpoint during travel.
// ----------------------------------------------------------------------------
void test_return_to_origin()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();

    // closed -> opening -> closed
    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    cap.open();
    closeSensor.setState(HIGH);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENING);

    closeSensor.setState(LOW); // returns to origin
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    // opened -> closing -> opened
    reset_fixtures();
    cap.setup();
    openSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENED);

    cap.close();
    openSensor.setState(HIGH);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSING);

    openSensor.setState(LOW); // returns to origin
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENED);
}

// ----------------------------------------------------------------------------
// 6. External movement from closed and opened.
// ----------------------------------------------------------------------------
void test_external_movement()
{
    // External opening from closed.
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();

    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    closeSensor.setState(HIGH); // no command, endpoint released
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENING);

    // External closing from opened.
    reset_fixtures();
    cap.setup();
    openSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENED);

    openSensor.setState(HIGH); // no command, endpoint released
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSING);
}

// ----------------------------------------------------------------------------
// 7. Command with both endpoints inactive.
// ----------------------------------------------------------------------------
void test_command_with_both_endpoints_inactive()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_UNKNOWN);

    cap.open();
    tick(cap, 0);
    assert_state(cap, GARAGE_STATE_OPENING);

    cap.close();
    tick(cap, 0);
    assert_state(cap, GARAGE_STATE_CLOSING);
}

// ----------------------------------------------------------------------------
// 8. Reverse direction during travel.
// ----------------------------------------------------------------------------
void test_reverse_during_travel()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();

    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    cap.open();
    closeSensor.setState(HIGH);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENING);

    // Reverse before reaching the open endpoint.
    cap.close();
    tick(cap, 0);
    assert_state(cap, GARAGE_STATE_CLOSING);

    // The open endpoint can still terminate the movement if reached first.
    openSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_OPENED);
}

// ----------------------------------------------------------------------------
// 9. Bounce shorter than 50 ms and stable change at or above debounce.
// ----------------------------------------------------------------------------
void test_bounce()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_UNKNOWN);

    // Short bounce: LOW for 30 ms then back to HIGH -> no state change.
    closeSensor.setState(LOW);
    tick(cap, 30);
    assert_state(cap, GARAGE_STATE_UNKNOWN);
    closeSensor.setState(HIGH);
    tick(cap, 30);
    assert_state(cap, GARAGE_STATE_UNKNOWN);

    // Stable change: LOW for the full debounce interval -> closed.
    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);

    // A shorter bounce in the opposite direction must not invert direction.
    closeSensor.setState(HIGH);
    tick(cap, 20);
    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_CLOSED);
}

// ----------------------------------------------------------------------------
// 10. Both sensors active produce unknown.
// ----------------------------------------------------------------------------
void test_both_sensors_active_unknown()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();

    openSensor.setState(LOW);
    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_state(cap, GARAGE_STATE_UNKNOWN);
}

// ----------------------------------------------------------------------------
// 11. Missing sensors and single-sensor configurations.
// ----------------------------------------------------------------------------
void test_missing_and_partial_sensors()
{
    // No sensors: commands publish movement without fabricated termination.
    reset_fixtures();
    {
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            nullptr, nullptr, &eventSink, 50);
        cap.setup();
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_UNKNOWN);

        cap.open();
        tick(cap, 0);
        assert_state(cap, GARAGE_STATE_OPENING);
        tick(cap, 500);
        assert_state(cap, GARAGE_STATE_OPENING); // no fabricated opened

        cap.close();
        tick(cap, 0);
        assert_state(cap, GARAGE_STATE_CLOSING);
        tick(cap, 500);
        assert_state(cap, GARAGE_STATE_CLOSING); // no fabricated closed
    }

    // Only open sensor: can confirm opened, never closed.
    reset_fixtures();
    {
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            &openSensor, nullptr, &eventSink, 50);
        cap.setup();
        openSensor.setState(LOW);
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_OPENED);

        cap.close();
        openSensor.setState(HIGH);
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_CLOSING);
        tick(cap, 500);
        assert_state(cap, GARAGE_STATE_CLOSING); // cannot fabricate closed
    }

    // Only close sensor: can confirm closed, never opened.
    reset_fixtures();
    {
        core::GarageControlCapability cap(
            "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
            nullptr, &closeSensor, &eventSink, 50);
        cap.setup();
        closeSensor.setState(LOW);
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_CLOSED);

        cap.open();
        closeSensor.setState(HIGH);
        tick(cap, 50);
        assert_state(cap, GARAGE_STATE_OPENING);
        tick(cap, 500);
        assert_state(cap, GARAGE_STATE_OPENING); // cannot fabricate opened
    }
}

// ----------------------------------------------------------------------------
// 12. Event order and absence of duplicate publications.
// ----------------------------------------------------------------------------
void test_event_order_and_no_duplicates()
{
    reset_fixtures();
    core::GarageControlCapability cap(
        "garage_test", 1, openAdapter, closeAdapter, stopUnlockAdapter, lockAdapter,
        &openSensor, &closeSensor, &eventSink, 50);
    cap.setup();
    tick(cap, 50);
    assert_event_count(0); // unknown is the initial value, no publication

    closeSensor.setState(LOW);
    tick(cap, 50);
    assert_event_count(1);
    assert_last_event(GARAGE_STATE_CLOSED);

    tick(cap, 100);
    assert_event_count(1); // no duplicate

    cap.open();
    closeSensor.setState(HIGH);
    tick(cap, 50);
    assert_event_count(2);
    assert_last_event(GARAGE_STATE_OPENING);

    openSensor.setState(LOW);
    tick(cap, 50);
    assert_event_count(3);
    assert_last_event(GARAGE_STATE_OPENED);

    TEST_ASSERT_EQUAL_STRING(GARAGE_STATE_CLOSED, eventSink.events[0].value.c_str());
    TEST_ASSERT_EQUAL_STRING(GARAGE_STATE_OPENING, eventSink.events[1].value.c_str());
    TEST_ASSERT_EQUAL_STRING(GARAGE_STATE_OPENED, eventSink.events[2].value.c_str());
}

void setup()
{
    delay(200);
    Serial.begin(115200);
    core::Log::setLogger(&logger);
    core::Time::setProvider(&timeProvider);

    UNITY_BEGIN();
    RUN_TEST(test_defaults_and_debounce_separation);
    RUN_TEST(test_initialization_combinations);
    RUN_TEST(test_commanded_open_and_symmetric_close);
    RUN_TEST(test_failed_start_keeps_origin);
    RUN_TEST(test_return_to_origin);
    RUN_TEST(test_external_movement);
    RUN_TEST(test_command_with_both_endpoints_inactive);
    RUN_TEST(test_reverse_during_travel);
    RUN_TEST(test_bounce);
    RUN_TEST(test_both_sensors_active_unknown);
    RUN_TEST(test_missing_and_partial_sensors);
    RUN_TEST(test_event_order_and_no_duplicates);
    UNITY_END();
}

void loop() {}
