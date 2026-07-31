#include <Arduino.h>
#include <unity.h>

extern "C" {
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"
}

#include "Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.h"
#include "Contracts/Common/StateResult.h"

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

using iotsmartsys::core::common::StateResult;
using iotsmartsys::platform::espressif::EspNvsBinaryCapabilityStateProvider;

static const char *NVS_NAMESPACE = "iotbcs";
static const char *NVS_KEY = "state";

void setUp(void)
{
    // Determinism: erase NVS so each test starts from an empty device.
    nvs_flash_erase();
    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_init());
}

void tearDown(void) {}

// BCS-011: first boot / erased NVS is a legitimate absence, not a failure.
void test_first_boot_is_absent()
{
    EspNvsBinaryCapabilityStateProvider provider;
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.loadSnapshot()));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

// BCS-002/BCS-013: save persists by (capability_name, type); a fresh provider
// instance loading the same NVS restores it (simulated reboot).
void test_save_persists_across_reboot()
{
    {
        EspNvsBinaryCapabilityStateProvider provider;
        TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.loadSnapshot()));
        TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.save("dev_Switch", "Switch", true)));
    }

    // New instance == new boot's cache, backed by the same NVS namespace.
    EspNvsBinaryCapabilityStateProvider reboot;
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(reboot.loadSnapshot()));

    bool isOn = false;
    TEST_ASSERT_TRUE(reboot.tryGet("dev_Switch", "Switch", isOn));
    TEST_ASSERT_TRUE(isOn);
}

// BCS-019/BCS-020: identity is the (capability_name, type) pair; same name with
// different type is a distinct, isolated record, and vice-versa.
void test_identity_isolation()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.loadSnapshot();

    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.save("dev_shared", "Switch", true)));
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.save("dev_shared", "Valve Actuator", false)));

    bool switchOn = false;
    bool valveOn = true;
    TEST_ASSERT_TRUE(provider.tryGet("dev_shared", "Switch", switchOn));
    TEST_ASSERT_TRUE(provider.tryGet("dev_shared", "Valve Actuator", valveOn));
    TEST_ASSERT_TRUE(switchOn);
    TEST_ASSERT_FALSE(valveOn);

    // Renaming (different capability_name) must not resolve to the old record.
    bool renamed = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_renamed", "Switch", renamed));
}

// BCS-006: no more than 8 active records; the 9th distinct identity is rejected.
void test_eight_record_limit()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.loadSnapshot();

    char name[24];
    for (int i = 0; i < 8; ++i)
    {
        snprintf(name, sizeof(name), "dev_cap_%d", i);
        TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.save(name, "Switch", (i % 2) == 0)));
    }

    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Overflow), static_cast<int>(provider.save("dev_cap_9th", "Switch", true)));

    // Re-saving an already-tracked identity must still succeed (not counted
    // as a new slot).
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.save("dev_cap_0", "Switch", false)));
}

// BCS-006/BCS-012: a truncated/incompatibly-sized blob is treated as absent.
void test_truncated_blob_is_rejected()
{
    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_init());
    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    const std::uint8_t truncated[4] = {1, 0, 0, 0};
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, truncated, sizeof(truncated)));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);

    EspNvsBinaryCapabilityStateProvider provider;
    const StateResult rc = provider.loadSnapshot();
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::StorageCorrupt), static_cast<int>(rc));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

// BCS-006/BCS-012: an unknown version is treated as absent, not applied.
void test_unknown_version_is_rejected()
{
    // Persist a well-formed record first, capturing the on-disk layout size.
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.loadSnapshot();
        provider.save("dev_Switch", "Switch", true);
    }

    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    size_t required = 0;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, nullptr, &required));
    std::vector<std::uint8_t> blob(required);
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, blob.data(), &required));

    // First field is the little-endian uint32_t version; corrupt it.
    std::uint32_t bogusVersion = 0xDEADBEEF;
    memcpy(blob.data(), &bogusVersion, sizeof(bogusVersion));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, blob.data(), blob.size()));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);

    EspNvsBinaryCapabilityStateProvider provider;
    const StateResult rc = provider.loadSnapshot();
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::StorageVersionMismatch), static_cast<int>(rc));

    bool isOn = false;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

// BCS-014: re-saving the same identity/value does not corrupt or drop other
// records already present in the snapshot.
void test_repeated_save_keeps_other_records()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.loadSnapshot();
    provider.save("dev_a", "Switch", true);
    provider.save("dev_b", "Light Actuator", false);
    provider.save("dev_a", "Switch", true); // repeat, same value

    bool aOn = false, bOn = true;
    TEST_ASSERT_TRUE(provider.tryGet("dev_a", "Switch", aOn));
    TEST_ASSERT_TRUE(provider.tryGet("dev_b", "Light Actuator", bOn));
    TEST_ASSERT_TRUE(aOn);
    TEST_ASSERT_FALSE(bOn);
}

// BCS-002: identities that would collide by prefix or exceed the internal
// buffer capacity must be rejected, never silently truncated.
void test_oversized_identity_is_rejected_not_truncated()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.loadSnapshot();

    std::string longName(64, 'x'); // beyond NAME_LEN(48)
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::InvalidArg),
                       static_cast<int>(provider.save(longName.c_str(), "Switch", true)));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet(longName.c_str(), "Switch", isOn));

    // A prefix-colliding shorter identity must remain distinct and unaffected.
    std::string shortName(40, 'x');
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok),
                       static_cast<int>(provider.save(shortName.c_str(), "Switch", false)));
    bool shortOn = true;
    TEST_ASSERT_TRUE(provider.tryGet(shortName.c_str(), "Switch", shortOn));
    TEST_ASSERT_FALSE(shortOn);
}

// BCS-006/BCS-012: mutating a single byte of the header or of an active
// record's region must be detected by the checksum, even though size and
// version stay unchanged; corrupted content is rejected wholesale.
void test_header_byte_corruption_is_rejected()
{
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.loadSnapshot();
        provider.save("dev_Switch", "Switch", true);
    }

    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    size_t required = 0;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, nullptr, &required));
    std::vector<std::uint8_t> blob(required);
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, blob.data(), &required));

    // Flip a byte inside the header (version field), preserving overall size.
    blob[0] ^= 0xFF;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, blob.data(), blob.size()));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);

    EspNvsBinaryCapabilityStateProvider provider;
    const StateResult rc = provider.loadSnapshot();
    TEST_ASSERT_NOT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(rc));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

void test_active_record_byte_corruption_is_rejected()
{
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.loadSnapshot();
        provider.save("dev_Switch", "Switch", true);
    }

    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    size_t required = 0;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, nullptr, &required));
    std::vector<std::uint8_t> blob(required);
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, blob.data(), &required));

    // Flip a byte past the header (version+checksum, 8 bytes), inside the
    // first record's region, preserving overall size and the version field.
    blob[8] ^= 0xFF;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, blob.data(), blob.size()));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);

    EspNvsBinaryCapabilityStateProvider provider;
    const StateResult rc = provider.loadSnapshot();
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::StorageCorrupt), static_cast<int>(rc));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_first_boot_is_absent);
    RUN_TEST(test_save_persists_across_reboot);
    RUN_TEST(test_identity_isolation);
    RUN_TEST(test_eight_record_limit);
    RUN_TEST(test_truncated_blob_is_rejected);
    RUN_TEST(test_unknown_version_is_rejected);
    RUN_TEST(test_repeated_save_keeps_other_records);
    RUN_TEST(test_oversized_identity_is_rejected_not_truncated);
    RUN_TEST(test_header_byte_corruption_is_rejected);
    RUN_TEST(test_active_record_byte_corruption_is_rejected);
    UNITY_END();
}

void loop() {}
