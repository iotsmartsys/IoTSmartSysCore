#include <Arduino.h>
#include <unity.h>

#include <string>
#include <vector>

#include "App/Managers/ProvisioningController.h"
#include "Core/Provisioning/DeviceConfig.h"
#include "Contracts/Logging/ILogger.h"

using namespace iotsmartsys;

namespace
{
    // Captures the diagnostic class emitted around the provisioning decision,
    // so success and failure can be told apart without a real reboot.
    class CapturingLogger : public core::ILogger
    {
    public:
        void logf(core::LogLevel level, const char *tag, const char *fmt, va_list args) override
        {
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            levels.push_back(level);
            messages.push_back(buffer);
            (void)tag;
        }

        void clear()
        {
            levels.clear();
            messages.clear();
        }

        bool hasLevel(core::LogLevel level) const
        {
            for (auto l : levels)
            {
                if (l == level)
                    return true;
            }
            return false;
        }

        std::vector<core::LogLevel> levels;
        std::vector<std::string> messages;
    };

    core::provisioning::DeviceConfig validConfig()
    {
        core::provisioning::DeviceConfig cfg{};
        cfg.wifi.ssid = "IoT_Test";
        cfg.wifi.password = "secret";
        cfg.wifi.profile = "primary";
        cfg.deviceApiUrl = "https://example.invalid/devices";
        cfg.deviceApiKey = "key";
        cfg.basicAuth = "auth";
        return cfg;
    }

    CapturingLogger logger;
} // namespace

void setUp(void) { logger.clear(); }
void tearDown(void) {}

// ---------------------------------------------------------------------------
// BCS-AC-025 — on success, the controlled restart and the success status/log
// happen only after save() returned successfully, and in that order.
// ---------------------------------------------------------------------------
void test_successful_save_schedules_restart_after_the_commit()
{
    std::vector<std::string> order;
    core::settings::Settings persisted;

    const bool result = app::ProvisioningController::completeProvisioning(
        validConfig(),
        [&order, &persisted](const core::settings::Settings &s)
        {
            order.push_back("save");
            persisted = s;
            return true;
        },
        [&order]()
        { order.push_back("restart"); },
        logger);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(2, order.size());
    TEST_ASSERT_EQUAL_STRING("save", order[0].c_str());
    TEST_ASSERT_EQUAL_STRING("restart", order[1].c_str());

    // The accepted configuration reached the persistence step intact.
    TEST_ASSERT_EQUAL_STRING("IoT_Test", persisted.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("primary", persisted.wifi.profile.c_str());
    TEST_ASSERT_EQUAL_STRING("IoT_Test", persisted.wifi.primary.ssid.c_str());
    TEST_ASSERT_FALSE(persisted.in_config_mode);

    TEST_ASSERT_FALSE(logger.hasLevel(core::LogLevel::Error));
}

// ---------------------------------------------------------------------------
// BCS-AC-025/BCS-026 — a failed save schedules no success restart, stays
// observable as a failure and leaves a further attempt possible.
// ---------------------------------------------------------------------------
void test_failed_save_never_schedules_a_success_restart()
{
    int restarts = 0;
    int saveAttempts = 0;

    const bool result = app::ProvisioningController::completeProvisioning(
        validConfig(),
        [&saveAttempts](const core::settings::Settings &)
        {
            saveAttempts++;
            return false;
        },
        [&restarts]()
        { restarts++; },
        logger);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(1, saveAttempts);
    TEST_ASSERT_EQUAL(0, restarts);
    // The failure is diagnosed as a failure, never logged as success.
    TEST_ASSERT_TRUE(logger.hasLevel(core::LogLevel::Error));

    // A further attempt remains possible and succeeds.
    logger.clear();
    const bool retry = app::ProvisioningController::completeProvisioning(
        validConfig(),
        [](const core::settings::Settings &)
        { return true; },
        [&restarts]()
        { restarts++; },
        logger);

    TEST_ASSERT_TRUE(retry);
    TEST_ASSERT_EQUAL(1, restarts);
}

// A missing saver is treated as a failed save, never as an implicit success.
void test_absent_saver_is_treated_as_failure()
{
    int restarts = 0;

    const bool result = app::ProvisioningController::completeProvisioning(
        validConfig(),
        nullptr,
        [&restarts]()
        { restarts++; },
        logger);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(0, restarts);
    TEST_ASSERT_TRUE(logger.hasLevel(core::LogLevel::Error));
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_successful_save_schedules_restart_after_the_commit);
    RUN_TEST(test_failed_save_never_schedules_a_success_restart);
    RUN_TEST(test_absent_saver_is_treated_as_failure);
    UNITY_END();
}

void loop() {}
