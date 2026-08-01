#include <Arduino.h>
#include <unity.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Core/Providers/ServiceManager.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Espressif/Providers/EspressifPlatformServiceRegistrar.h"

using namespace iotsmartsys;

// Address observed by a task that stands in for the BLE/provisioning context,
// which is where the duplicated graph was originally constructed.
static volatile core::ServiceManager *g_fromTask = nullptr;
static volatile bool g_taskDone = false;

static void probeTaskEntry(void *)
{
    g_fromTask = &core::ServiceManager::instance();
    g_taskDone = true;
    vTaskDelete(nullptr);
}

void setUp(void) {}
void tearDown(void) {}

// ---------------------------------------------------------------------------
// BCS-AC-023 — init() and instance() resolve the same object; the graph is
// constructed once, the platform services are registered once and the binary
// snapshot is read once per boot, including when reached from a task context.
// ---------------------------------------------------------------------------
void test_accessors_converge_on_a_single_instance()
{
    // Bootstrap: init() completes before anything concurrent runs.
    core::ServiceManager &fromInit = core::ServiceManager::init();
    core::ServiceManager &fromInstance = core::ServiceManager::instance();

    TEST_ASSERT_EQUAL_PTR(&fromInit, &fromInstance);
    TEST_ASSERT_EQUAL_PTR(&fromInit, &core::ServiceManager::init());
    TEST_ASSERT_EQUAL_PTR(&fromInit, &core::ServiceManager::instance());

    // Exactly one construction, one platform registration, one snapshot read.
    TEST_ASSERT_EQUAL_UINT32(1u, platform::espressif::EspressifPlatformServiceRegistrar::registrationCount());
    TEST_ASSERT_EQUAL_UINT32(1u, platform::espressif::EspressifPlatformServiceRegistrar::snapshotLoadCount());
}

void test_task_context_reuses_the_bootstrapped_graph()
{
    core::ServiceManager &fromInit = core::ServiceManager::init();
    const std::uint32_t registrationsBefore =
        platform::espressif::EspressifPlatformServiceRegistrar::registrationCount();
    const std::uint32_t loadsBefore =
        platform::espressif::EspressifPlatformServiceRegistrar::snapshotLoadCount();

    g_fromTask = nullptr;
    g_taskDone = false;
    TEST_ASSERT_EQUAL(pdPASS, xTaskCreate(&probeTaskEntry, "sm_probe", 4096, nullptr, 4, nullptr));

    const uint32_t startedAt = millis();
    while (!g_taskDone && (millis() - startedAt) < 2000)
    {
        delay(5);
    }
    TEST_ASSERT_TRUE(g_taskDone);

    // The provisioning-side accessor reuses the bootstrapped graph: no second
    // construction, no re-registration, no second snapshot read.
    TEST_ASSERT_EQUAL_PTR(&fromInit, const_cast<core::ServiceManager *>(g_fromTask));
    TEST_ASSERT_EQUAL_UINT32(registrationsBefore,
                             platform::espressif::EspressifPlatformServiceRegistrar::registrationCount());
    TEST_ASSERT_EQUAL_UINT32(loadsBefore,
                             platform::espressif::EspressifPlatformServiceRegistrar::snapshotLoadCount());
}

// BCS-AC-023/BCS-029: the writer is activated once, after init() returned, and
// activating again never creates a second worker.
void test_writer_activation_happens_once_after_init()
{
    core::ServiceManager &manager = core::ServiceManager::init();

    auto *provider = manager.binaryCapabilityStateProvider();
    TEST_ASSERT_NOT_NULL(provider);
    TEST_ASSERT_FALSE(provider->writerStatus().available);

    TEST_ASSERT_EQUAL(static_cast<int>(core::common::StateResult::Ok),
                      static_cast<int>(manager.activateBinaryStateWriter()));
    TEST_ASSERT_TRUE(provider->writerStatus().available);

    TEST_ASSERT_EQUAL(static_cast<int>(core::common::StateResult::Ok),
                      static_cast<int>(manager.activateBinaryStateWriter()));
    TEST_ASSERT_TRUE(provider->writerStatus().available);
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_accessors_converge_on_a_single_instance);
    RUN_TEST(test_task_context_reuses_the_bootstrapped_graph);
    RUN_TEST(test_writer_activation_happens_once_after_init);
    UNITY_END();
}

void loop() {}
