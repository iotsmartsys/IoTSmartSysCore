#include <Arduino.h>
#include <unity.h>

#include "Contracts/Capabilities/Managers/CapabilityManager.h"

using namespace iotsmartsys;

namespace
{
    class NoopLogger final : public core::ILogger
    {
    public:
        void logf(core::LogLevel, const char *, const char *, va_list) override {}
    };

    class ReadyGate final : public core::settings::ISettingsGate
    {
    public:
        core::settings::SettingsReadyLevel level() const override
        {
            return core::settings::SettingsReadyLevel::Available;
        }
        void signalAvailable() override {}
        void signalSynced() override {}
        void signalSyncing() override {}
        void signalError(core::common::StateResult) override {}
        void setLevel(core::settings::SettingsReadyLevel, core::common::StateResult) override {}
        core::common::StateResult runWhenReady(
            core::settings::SettingsReadyLevel,
            core::settings::SettingsGateCallback callback,
            void *context) override
        {
            callback(core::settings::SettingsReadyLevel::Available, context);
            return core::common::StateResult::Ok;
        }
    };

    class EmptySettings final : public core::settings::IReadOnlySettingsProvider
    {
    public:
        bool hasCurrent() const override { return false; }
        bool copyCurrent(core::settings::Settings &) const override { return false; }
        const core::settings::Settings getSettings() const override { return {}; }
    };

    int setupOrder[9]{};
    int handleOrder[9]{};
    int setupCursor = 0;
    int handleCursor = 0;

    class TrackingCapability final : public core::ICapability
    {
    public:
        explicit TrackingCapability(int index)
            : core::ICapability("tracking", "Tracking", ""), index_(index) {}

        void setup() override
        {
            setupCalls++;
            setupOrder[setupCursor++] = index_;
        }

        void handle() override
        {
            handleCalls++;
            handleOrder[handleCursor++] = index_;
        }

        int setupCalls{0};
        int handleCalls{0};

    private:
        int index_;
    };
}

void setUp()
{
    setupCursor = 0;
    handleCursor = 0;
}

void tearDown() {}

// CAP-AC-003 — a manager built from nine registered capabilities traverses
// each one exactly once and preserves registration order without extra tasks.
void test_nine_capabilities_receive_setup_and_cooperative_handle_in_order()
{
    TrackingCapability capabilities[] = {
        TrackingCapability(0),
        TrackingCapability(1),
        TrackingCapability(2),
        TrackingCapability(3),
        TrackingCapability(4),
        TrackingCapability(5),
        TrackingCapability(6),
        TrackingCapability(7),
        TrackingCapability(8)};
    core::ICapability *items[9]{};
    for (int i = 0; i < 9; ++i)
    {
        items[i] = &capabilities[i];
    }

    ReadyGate gate;
    NoopLogger logger;
    EmptySettings settings;
    core::CapabilityManager manager(items, 9, gate, logger, settings);
    manager.setup();
    manager.handle();

    TEST_ASSERT_EQUAL(9, setupCursor);
    TEST_ASSERT_EQUAL(9, handleCursor);
    for (int i = 0; i < 9; ++i)
    {
        TEST_ASSERT_EQUAL(1, capabilities[i].setupCalls);
        TEST_ASSERT_EQUAL(1, capabilities[i].handleCalls);
        TEST_ASSERT_EQUAL(i, setupOrder[i]);
        TEST_ASSERT_EQUAL(i, handleOrder[i]);
    }
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_nine_capabilities_receive_setup_and_cooperative_handle_in_order);
    UNITY_END();
}

void loop() {}
