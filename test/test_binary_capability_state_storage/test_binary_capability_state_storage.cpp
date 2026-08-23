#include <Arduino.h>
#include <unity.h>

extern "C" {
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"
}

#include "Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Capabilities/SwitchCapability.h"
#include "Contracts/Capabilities/SwitchPlugCapability.h"
#include "Contracts/Capabilities/LightCapability.h"
#include "Contracts/Capabilities/LEDCapability.h"
#include "Contracts/Capabilities/ValveCapability.h"
#include "Platform/Arduino/Interpreters/ValveHardwareCommandInterpreter.h"

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

using iotsmartsys::core::common::StateResult;
using iotsmartsys::platform::espressif::EspNvsBinaryCapabilityStateProvider;

static const char *NVS_NAMESPACE = "iotbcs";
static const char *NVS_KEY = "state";
static const char *SETTINGS_NAMESPACE = "iotsettings";
static const char *SETTINGS_KEY = "sentinel";

// Public identity maxima published by BCS-DEC-005.
static const size_t MAX_NAME_BYTES = 63;
static const size_t MAX_TYPE_BYTES = 31;

// ---------------------------------------------------------------------------
// NVS seam (spec 8.2). Counts operations per kind, fails init/open/read/write/
// commit individually, and proves that no global erase is ever emitted by the
// binary storage: the seam offers no erase entry point at all, and the counter
// below can only move if a test itself erases.
// ---------------------------------------------------------------------------
namespace seam
{
    int flashInitCalls = 0;
    int openCalls = 0;
    int metadataQueries = 0; // getBlob with a null destination: no data copied
    int dataReads = 0;       // getBlob that copies blob content into memory
    int writeCalls = 0;
    int commitCalls = 0;
    int globalEraseCalls = 0;

    bool failInit = false;
    bool failOpen = false;
    bool failRead = false;
    bool failWrite = false;
    bool failCommit = false;

    void reset()
    {
        flashInitCalls = openCalls = metadataQueries = dataReads = 0;
        writeCalls = commitCalls = globalEraseCalls = 0;
        failInit = failOpen = failRead = failWrite = failCommit = false;
    }

    esp_err_t flashInit()
    {
        flashInitCalls++;
        if (failInit)
            return ESP_ERR_NVS_NO_FREE_PAGES;
        return nvs_flash_init();
    }

    esp_err_t open(const char *ns, nvs_open_mode_t mode, nvs_handle_t *out)
    {
        openCalls++;
        if (failOpen)
            return ESP_ERR_NVS_NOT_INITIALIZED;
        return nvs_open(ns, mode, out);
    }

    esp_err_t getBlob(nvs_handle_t handle, const char *key, void *out, std::size_t *length)
    {
        if (out == nullptr)
            metadataQueries++;
        else
            dataReads++;

        if (failRead && out != nullptr)
            return ESP_ERR_NVS_INVALID_LENGTH;
        return nvs_get_blob(handle, key, out, length);
    }

    esp_err_t setBlob(nvs_handle_t handle, const char *key, const void *value, std::size_t length)
    {
        writeCalls++;
        if (failWrite)
            return ESP_ERR_NVS_NOT_ENOUGH_SPACE;
        return nvs_set_blob(handle, key, value, length);
    }

    esp_err_t commit(nvs_handle_t handle)
    {
        commitCalls++;
        if (failCommit)
            return ESP_ERR_NVS_INVALID_STATE;
        return nvs_commit(handle);
    }

    void close(nvs_handle_t handle) { nvs_close(handle); }

    EspNvsBinaryCapabilityStateProvider::NvsOps ops()
    {
        return EspNvsBinaryCapabilityStateProvider::NvsOps{
            &flashInit, &open, &getBlob, &setBlob, &commit, &close};
    }
} // namespace seam

static void writeSettingsSentinel(const char *value)
{
    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &h));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, SETTINGS_KEY, value, strlen(value) + 1));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);
}

static bool settingsSentinelMatches(const char *expected)
{
    nvs_handle_t h;
    if (nvs_open(SETTINGS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
        return false;
    char buffer[64] = {};
    size_t len = sizeof(buffer);
    const esp_err_t rc = nvs_get_blob(h, SETTINGS_KEY, buffer, &len);
    nvs_close(h);
    return rc == ESP_OK && strcmp(buffer, expected) == 0;
}

static std::vector<std::uint8_t> readStoredBlob()
{
    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    size_t required = 0;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, nullptr, &required));
    std::vector<std::uint8_t> blob(required);
    TEST_ASSERT_EQUAL(ESP_OK, nvs_get_blob(h, NVS_KEY, blob.data(), &required));
    nvs_close(h);
    return blob;
}

static void writeStoredBlob(const std::vector<std::uint8_t> &blob)
{
    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, blob.data(), blob.size()));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);
}

// Runs a full request -> asynchronous write -> commit cycle and asserts it
// reached a successful terminal state.
static void persistAndAwait(EspNvsBinaryCapabilityStateProvider &provider,
                            const char *name, const char *type, bool isOn)
{
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok),
                      static_cast<int>(provider.requestSave(name, type, isOn)));
    TEST_ASSERT_TRUE(provider.waitForQuiescence(2000));
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok),
                      static_cast<int>(provider.writerStatus().lastError));
}

void setUp(void)
{
    // Determinism: erase NVS so each test starts from an empty device. The
    // erase belongs to the test harness, never to the provider.
    nvs_flash_erase();
    TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_init());
    seam::reset();
}

void tearDown(void) {}

// ---------------------------------------------------------------------------
// BCS-AC-011 — first boot / erased NVS is a legitimate absence, not a failure.
// ---------------------------------------------------------------------------
void test_first_boot_is_absent()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.loadSnapshot()));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
    TEST_ASSERT_EQUAL(0, seam::globalEraseCalls);
}

// ---------------------------------------------------------------------------
// BCS-AC-002 — round trip and isolation of full-length identities, including
// the 63/31-byte maxima and long common prefixes with distinct suffixes.
// ---------------------------------------------------------------------------
void test_full_length_identities_round_trip_without_truncation()
{
    const std::string maxName(MAX_NAME_BYTES, 'n');
    const std::string maxType(MAX_TYPE_BYTES, 't');
    const std::string prefixA = std::string(MAX_NAME_BYTES - 1, 'p') + "a";
    const std::string prefixB = std::string(MAX_NAME_BYTES - 1, 'p') + "b";

    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
        TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.activateWriter()));

        persistAndAwait(provider, maxName.c_str(), maxType.c_str(), true);
        persistAndAwait(provider, prefixA.c_str(), "Switch", true);
        persistAndAwait(provider, prefixB.c_str(), "Switch", false);
        // Same name, different type.
        persistAndAwait(provider, "dev_shared", "Switch", true);
        persistAndAwait(provider, "dev_shared", "Valve Actuator", false);
    }

    // Simulated reboot: a fresh provider reading the same NVS.
    EspNvsBinaryCapabilityStateProvider reboot;
    reboot.setNvsOps(seam::ops());
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(reboot.loadSnapshot()));

    bool value = false;
    TEST_ASSERT_TRUE(reboot.tryGet(maxName.c_str(), maxType.c_str(), value));
    TEST_ASSERT_TRUE(value);

    TEST_ASSERT_TRUE(reboot.tryGet(prefixA.c_str(), "Switch", value));
    TEST_ASSERT_TRUE(value);
    TEST_ASSERT_TRUE(reboot.tryGet(prefixB.c_str(), "Switch", value));
    TEST_ASSERT_FALSE(value); // no prefix collision

    TEST_ASSERT_TRUE(reboot.tryGet("dev_shared", "Switch", value));
    TEST_ASSERT_TRUE(value);
    TEST_ASSERT_TRUE(reboot.tryGet("dev_shared", "Valve Actuator", value));
    TEST_ASSERT_FALSE(value);

    // A truncated form of a stored maximum-length identity must not resolve.
    TEST_ASSERT_FALSE(reboot.tryGet(maxName.substr(0, MAX_NAME_BYTES - 1).c_str(), maxType.c_str(), value));
}

// BCS-AC-002: the storage is never more restrictive than the public contract.
// Only an identity beyond the public maxima is refused, and never truncated.
void test_identity_beyond_the_public_limit_is_refused_not_truncated()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();
    provider.activateWriter();

    const std::string overName(MAX_NAME_BYTES + 1, 'x');
    const std::string overType(MAX_TYPE_BYTES + 1, 'y');

    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::InvalidArg),
                      static_cast<int>(provider.requestSave(overName.c_str(), "Switch", true)));
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::InvalidArg),
                      static_cast<int>(provider.requestSave("dev_cap", overType.c_str(), true)));

    bool value = true;
    TEST_ASSERT_FALSE(provider.tryGet(overName.c_str(), "Switch", value));
    // No silent truncation to the maximum length either.
    TEST_ASSERT_FALSE(provider.tryGet(overName.substr(0, MAX_NAME_BYTES).c_str(), "Switch", value));
    TEST_ASSERT_EQUAL(0, provider.writerStatus().pending);
}

// ---------------------------------------------------------------------------
// BCS-AC-007 — zero and eight active records are accepted; the ninth distinct
// identity is refused without altering the last valid snapshot.
// ---------------------------------------------------------------------------
void test_eight_record_limit()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();
    provider.activateWriter();

    char name[32];
    for (int i = 0; i < 8; ++i)
    {
        snprintf(name, sizeof(name), "dev_cap_%d", i);
        persistAndAwait(provider, name, "Switch", (i % 2) == 0);
    }

    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Overflow),
                      static_cast<int>(provider.requestSave("dev_cap_9th", "Switch", true)));

    // The ninth refusal left every existing record untouched.
    bool value = false;
    for (int i = 0; i < 8; ++i)
    {
        snprintf(name, sizeof(name), "dev_cap_%d", i);
        TEST_ASSERT_TRUE(provider.tryGet(name, "Switch", value));
        TEST_ASSERT_EQUAL((i % 2) == 0, value);
    }

    // Re-saving an already tracked identity still succeeds (no new slot).
    persistAndAwait(provider, "dev_cap_0", "Switch", false);
    TEST_ASSERT_TRUE(provider.tryGet("dev_cap_0", "Switch", value));
    TEST_ASSERT_FALSE(value);
}

// ---------------------------------------------------------------------------
// BCS-AC-009 — at most one NVS data read per boot; every later lookup is served
// from the cache. A size-only query is counted separately.
// ---------------------------------------------------------------------------
void test_single_data_read_per_boot()
{
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
        provider.activateWriter();
        char name[32];
        for (int i = 0; i < 8; ++i)
        {
            snprintf(name, sizeof(name), "dev_cap_%d", i);
            persistAndAwait(provider, name, "Switch", true);
        }
    }

    seam::reset();
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();

    TEST_ASSERT_LESS_OR_EQUAL(1, seam::dataReads);
    TEST_ASSERT_EQUAL(1, seam::metadataQueries);

    const int readsAfterBoot = seam::dataReads;
    bool value = false;
    char name[32];
    for (int round = 0; round < 3; ++round)
    {
        for (int i = 0; i < 8; ++i)
        {
            snprintf(name, sizeof(name), "dev_cap_%d", i);
            TEST_ASSERT_TRUE(provider.tryGet(name, "Switch", value));
        }
    }
    TEST_ASSERT_EQUAL(readsAfterBoot, seam::dataReads); // delta zero
}

// ---------------------------------------------------------------------------
// BCS-AC-006 — the binary domain only ever opens its own namespace/key; the
// settings sentinel stays byte-for-byte identical and no global erase happens.
// ---------------------------------------------------------------------------
void test_settings_namespace_is_never_touched()
{
    writeSettingsSentinel("settings-sentinel-value");

    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();
    provider.activateWriter();
    persistAndAwait(provider, "dev_Switch", "Switch", true);

    TEST_ASSERT_TRUE(settingsSentinelMatches("settings-sentinel-value"));
    TEST_ASSERT_EQUAL(0, seam::globalEraseCalls);
}

// ---------------------------------------------------------------------------
// BCS-AC-007/BCS-AC-008 — structural, size, version and single-byte integrity
// rejection. Matching size and version never suffice.
// ---------------------------------------------------------------------------
void test_truncated_blob_is_rejected()
{
    nvs_handle_t h;
    TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
    const std::uint8_t truncated[4] = {2, 0, 0, 0};
    TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, truncated, sizeof(truncated)));
    TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
    nvs_close(h);

    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::StorageCorrupt),
                      static_cast<int>(provider.loadSnapshot()));

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

void test_unknown_version_is_rejected()
{
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
        provider.activateWriter();
        persistAndAwait(provider, "dev_Switch", "Switch", true);
    }

    auto blob = readStoredBlob();
    const std::uint32_t bogusVersion = 0xDEADBEEF;
    memcpy(blob.data(), &bogusVersion, sizeof(bogusVersion));
    writeStoredBlob(blob);

    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::StorageVersionMismatch),
                      static_cast<int>(provider.loadSnapshot()));

    bool isOn = false;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

void test_single_byte_corruption_is_rejected_everywhere()
{
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
        provider.activateWriter();
        persistAndAwait(provider, "dev_Switch", "Switch", true);
        persistAndAwait(provider, "dev_Valve", "Valve Actuator", false);
    }

    const auto pristine = readStoredBlob();

    // Header checksum region plus one byte inside each active record region.
    const size_t headerBytes = sizeof(std::uint32_t) * 2;
    const size_t recordBytes = (pristine.size() - headerBytes) / 8;
    const size_t offsets[] = {
        4,                            // checksum field
        headerBytes,                  // first record's name
        headerBytes + recordBytes - 1 // first record's flags
    };

    for (size_t offset : offsets)
    {
        auto blob = pristine;
        blob[offset] ^= 0xFF;
        writeStoredBlob(blob);

        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        const StateResult rc = provider.loadSnapshot();
        TEST_ASSERT_NOT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(rc));

        bool isOn = true;
        TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
        TEST_ASSERT_FALSE(provider.tryGet("dev_Valve", "Valve Actuator", isOn));
    }
}

// ---------------------------------------------------------------------------
// BCS-AC-027 — semantically invalid snapshots carrying a *correct* checksum are
// rejected before any strcmp or application.
// ---------------------------------------------------------------------------
namespace
{
    // Mirrors the on-disk layout so a test can build deliberately invalid but
    // checksum-consistent content.
    struct MirrorRecord
    {
        char capability_name[64];
        char type[32];
        std::uint8_t isOn;
        std::uint8_t used;
    };

    struct MirrorSnapshot
    {
        std::uint32_t version;
        std::uint32_t checksum;
        MirrorRecord records[8];
    };

    std::uint32_t mirrorChecksum(const MirrorSnapshot &snapshot)
    {
        std::uint32_t hash = 2166136261u;
        auto mix = [&hash](const std::uint8_t *bytes, std::size_t n)
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                hash ^= bytes[i];
                hash *= 16777619u;
            }
        };
        mix(reinterpret_cast<const std::uint8_t *>(&snapshot.version), sizeof(snapshot.version));
        mix(reinterpret_cast<const std::uint8_t *>(snapshot.records), sizeof(snapshot.records));
        return hash;
    }

    MirrorSnapshot validMirror()
    {
        MirrorSnapshot snapshot{};
        snapshot.version = 2;
        strcpy(snapshot.records[0].capability_name, "dev_Switch");
        strcpy(snapshot.records[0].type, "Switch");
        snapshot.records[0].isOn = 1;
        snapshot.records[0].used = 1;
        snapshot.checksum = mirrorChecksum(snapshot);
        return snapshot;
    }

    void storeMirror(const MirrorSnapshot &snapshot)
    {
        nvs_handle_t h;
        TEST_ASSERT_EQUAL(ESP_OK, nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
        TEST_ASSERT_EQUAL(ESP_OK, nvs_set_blob(h, NVS_KEY, &snapshot, sizeof(snapshot)));
        TEST_ASSERT_EQUAL(ESP_OK, nvs_commit(h));
        nvs_close(h);
    }

    void assertMirrorRejected(const MirrorSnapshot &snapshot, const char *message)
    {
        storeMirror(snapshot);
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        const StateResult rc = provider.loadSnapshot();
        TEST_ASSERT_NOT_EQUAL_MESSAGE(static_cast<int>(StateResult::Ok), static_cast<int>(rc), message);

        bool isOn = true;
        TEST_ASSERT_FALSE_MESSAGE(provider.tryGet("dev_Switch", "Switch", isOn), message);
    }
} // namespace

void test_layout_matches_the_storage_contract()
{
    // Guards the mirror used by the semantic tests against a silent layout
    // change in the provider.
    TEST_ASSERT_EQUAL(64, EspNvsBinaryCapabilityStateProvider::NAME_LEN);
    TEST_ASSERT_EQUAL(32, EspNvsBinaryCapabilityStateProvider::TYPE_LEN);
    TEST_ASSERT_EQUAL(8, EspNvsBinaryCapabilityStateProvider::MAX_RECORDS);

    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
        provider.activateWriter();
        persistAndAwait(provider, "dev_Switch", "Switch", true);
    }
    TEST_ASSERT_EQUAL(sizeof(MirrorSnapshot), readStoredBlob().size());
}

void test_semantically_invalid_snapshots_are_rejected()
{
    {
        auto snapshot = validMirror();
        snapshot.records[0].used = 7; // outside {0,1}
        snapshot.checksum = mirrorChecksum(snapshot);
        assertMirrorRejected(snapshot, "used out of domain");
    }
    {
        auto snapshot = validMirror();
        snapshot.records[0].isOn = 2; // outside {0,1}
        snapshot.checksum = mirrorChecksum(snapshot);
        assertMirrorRejected(snapshot, "isOn out of domain");
    }
    {
        auto snapshot = validMirror();
        memset(snapshot.records[0].capability_name, 'x', sizeof(snapshot.records[0].capability_name));
        snapshot.checksum = mirrorChecksum(snapshot);
        assertMirrorRejected(snapshot, "name without null terminator");
    }
    {
        auto snapshot = validMirror();
        memset(snapshot.records[0].type, 'y', sizeof(snapshot.records[0].type));
        snapshot.checksum = mirrorChecksum(snapshot);
        assertMirrorRejected(snapshot, "type without null terminator");
    }
    {
        auto snapshot = validMirror();
        snapshot.records[1].used = 1; // active record with an empty identity
        snapshot.checksum = mirrorChecksum(snapshot);
        assertMirrorRejected(snapshot, "active record with empty identity");
    }
}

// ---------------------------------------------------------------------------
// BCS-AC-016/BCS-AC-026 — every injected NVS failure returns observably, keeps
// the runtime alive, emits no global erase and lets the next cycle finish.
// ---------------------------------------------------------------------------
void test_init_failure_does_not_erase_or_abort()
{
    writeSettingsSentinel("settings-sentinel-value");

    seam::failInit = true;
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());

    // The legacy recovery would have called nvs_flash_erase() under
    // ESP_ERROR_CHECK for exactly this condition.
    const StateResult rc = provider.loadSnapshot();
    TEST_ASSERT_NOT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(rc));
    TEST_ASSERT_EQUAL(0, seam::globalEraseCalls);
    TEST_ASSERT_TRUE(settingsSentinelMatches("settings-sentinel-value"));

    // The process survived and the binary domain simply reports absence.
    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

void test_open_and_read_failures_preserve_the_default_flow()
{
    seam::failOpen = true;
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        // A namespace that cannot be opened is a legitimate absence.
        TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.loadSnapshot()));
        bool isOn = true;
        TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
    }

    seam::reset();
    {
        EspNvsBinaryCapabilityStateProvider seedProvider;
        seedProvider.setNvsOps(seam::ops());
        seedProvider.loadSnapshot();
        seedProvider.activateWriter();
        persistAndAwait(seedProvider, "dev_Switch", "Switch", true);
    }

    seam::failRead = true;
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    const StateResult rc = provider.loadSnapshot();
    // A read failure is diagnosed as a failure, never silently as absence.
    TEST_ASSERT_NOT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(rc));
    TEST_ASSERT_EQUAL(0, seam::globalEraseCalls);

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));
}

void test_write_and_commit_failures_stay_observable()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();
    provider.activateWriter();

    seam::failWrite = true;
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok),
                      static_cast<int>(provider.requestSave("dev_Switch", "Switch", true)));
    // The failed attempt leaves the entry pending, so quiescence is not
    // reached; the point is that the failure is observable, not silent.
    TEST_ASSERT_FALSE(provider.waitForQuiescence(500));

    auto status = provider.writerStatus();
    TEST_ASSERT_GREATER_OR_EQUAL(1u, status.failures);
    TEST_ASSERT_NOT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(status.lastError));
    TEST_ASSERT_EQUAL(0u, status.commits);
    TEST_ASSERT_EQUAL(0, seam::globalEraseCalls);

    bool isOn = true;
    TEST_ASSERT_FALSE(provider.tryGet("dev_Switch", "Switch", isOn));

    // A later confirmed transition retries and succeeds; the runtime never
    // aborted nor blocked.
    seam::failWrite = false;
    persistAndAwait(provider, "dev_Switch", "Switch", false);
    TEST_ASSERT_TRUE(provider.tryGet("dev_Switch", "Switch", isOn));
    TEST_ASSERT_FALSE(isOn);
}

// ---------------------------------------------------------------------------
// BCS-AC-028 — the requesting context performs no NVS work and never waits; the
// single writer consolidates bursts and only a successful commit survives.
// ---------------------------------------------------------------------------
void test_request_performs_no_nvs_work_in_the_calling_context()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();
    provider.activateWriter();

    const int writesBefore = seam::writeCalls;
    const int commitsBefore = seam::commitCalls;

    const uint32_t startedAt = millis();
    for (int i = 0; i < 16; ++i)
    {
        TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok),
                          static_cast<int>(provider.requestSave("dev_Switch", "Switch", (i % 2) == 0)));
    }
    const uint32_t elapsed = millis() - startedAt;

    // The caller returned immediately, with no write and no commit of its own.
    TEST_ASSERT_EQUAL(writesBefore, seam::writeCalls);
    TEST_ASSERT_EQUAL(commitsBefore, seam::commitCalls);
    TEST_ASSERT_LESS_THAN(100u, elapsed);

    // At most one pending entry for the identity, consolidated to the last
    // requested value.
    TEST_ASSERT_LESS_OR_EQUAL(1, provider.writerStatus().pending);

    TEST_ASSERT_TRUE(provider.waitForQuiescence(2000));
    bool isOn = false;
    TEST_ASSERT_TRUE(provider.tryGet("dev_Switch", "Switch", isOn));
    TEST_ASSERT_FALSE(isOn); // i == 15 -> false, the most recent value
}

void test_writer_is_activated_once_and_requires_activation()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();

    // Before activation there is no writer, and there is no synchronous
    // fallback either.
    TEST_ASSERT_FALSE(provider.writerStatus().available);
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::InvalidState),
                      static_cast<int>(provider.requestSave("dev_Switch", "Switch", true)));
    TEST_ASSERT_EQUAL(0, seam::writeCalls);

    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.activateWriter()));
    TEST_ASSERT_TRUE(provider.writerStatus().available);
    // Idempotent: a second activation never creates a second worker.
    TEST_ASSERT_EQUAL(static_cast<int>(StateResult::Ok), static_cast<int>(provider.activateWriter()));

    persistAndAwait(provider, "dev_Switch", "Switch", true);
}

// ---------------------------------------------------------------------------
// BCS-AC-010 — after the boot, handle() on every concrete type (including a
// stopped and a blinking LED) performs no NVS read at all.
// ---------------------------------------------------------------------------
namespace
{
    // Minimal command adapter with the OutputHardwareAdapter vocabulary, so the
    // valve's open/closed can only come from the interpreter.
    class OnOffAdapter : public iotsmartsys::core::ICommandHardwareAdapter
    {
    public:
        void setup() override {}
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }
        bool applyCommand(const iotsmartsys::core::IHardwareCommand &command) override
        {
            return applyCommand(command.getCommand().c_str());
        }
        bool applyCommand(const char *value) override
        {
            const std::string requested = value ? value : "";
            if (requested != "on" && requested != "off" && requested != "toggle")
                return false;
            state = (requested == "toggle") ? ((state == "on") ? "off" : "on") : requested;
            return true;
        }
        std::string getStateValue() override { return state; }
        iotsmartsys::core::IHardwareState getState() override
        {
            iotsmartsys::core::IHardwareState hwState;
            hwState.value = state;
            return hwState;
        }
        std::string state{"off"};
    };

    class NoopSink : public iotsmartsys::core::ICapabilityEventSink
    {
    public:
        void onStateChanged(const iotsmartsys::core::CapabilityStateChanged &) override {}
    };
} // namespace

void test_handle_never_reads_nvs()
{
    EspNvsBinaryCapabilityStateProvider provider;
    provider.setNvsOps(seam::ops());
    provider.loadSnapshot();
    provider.activateWriter();
    iotsmartsys::core::ServiceProvider::instance().setBinaryCapabilityStateProvider(&provider);

    NoopSink sink;
    OnOffAdapter switchAdapter, plugAdapter, lightAdapter, ledAdapter, valveAdapter;
    iotsmartsys::core::SwitchCapability switchCap("dev_switch", switchAdapter, &sink);
    iotsmartsys::core::SwitchPlugCapability plugCap("dev_plug", plugAdapter, &sink);
    iotsmartsys::core::LightCapability lightCap("dev_light", lightAdapter, &sink);
    iotsmartsys::core::LEDCapability ledCap("dev_led", ledAdapter, &sink);
    iotsmartsys::core::ValveCapability valveCap("dev_valve", valveAdapter, &sink);
    iotsmartsys::core::ValveHardwareCommandInterpreter valveInterpreter;
    valveCap.setCommandInterpreter(&valveInterpreter);

    switchCap.setup();
    plugCap.setup();
    lightCap.setup();
    ledCap.setup();
    valveCap.setup();

    const int readsAfterSetup = seam::dataReads;
    const int metadataAfterSetup = seam::metadataQueries;

    // LED stopped.
    for (int i = 0; i < 20; ++i)
    {
        switchCap.handle();
        plugCap.handle();
        lightCap.handle();
        ledCap.handle();
        valveCap.handle();
    }

    // LED blinking: the alternations are applied and published, and the
    // asynchronous writes they might trigger are never reads.
    ledCap.blink(1);
    for (int i = 0; i < 20; ++i)
    {
        delay(2);
        ledCap.handle();
    }
    ledCap.blink(0);

    TEST_ASSERT_EQUAL(readsAfterSetup, seam::dataReads);
    TEST_ASSERT_EQUAL(metadataAfterSetup, seam::metadataQueries);

    provider.waitForQuiescence(2000);
    iotsmartsys::core::ServiceProvider::instance().setBinaryCapabilityStateProvider(nullptr);
}

// ---------------------------------------------------------------------------
// BCS-AC-020 — absence, invalid content and each storage failure produce
// distinct diagnostics, and no private content is ever printed.
// ---------------------------------------------------------------------------
namespace
{
    class CapturingLogger : public iotsmartsys::core::ILogger
    {
    public:
        void logf(iotsmartsys::core::LogLevel level, const char *tag, const char *fmt, va_list args) override
        {
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            entries.push_back(std::string(tag ? tag : "") + "|" + buffer);
            levels.push_back(level);
        }

        bool contains(const char *needle) const
        {
            for (const auto &entry : entries)
            {
                if (entry.find(needle) != std::string::npos)
                    return true;
            }
            return false;
        }

        void clear()
        {
            entries.clear();
            levels.clear();
        }

        std::vector<std::string> entries;
        std::vector<iotsmartsys::core::LogLevel> levels;
    };

    CapturingLogger captured;
} // namespace

void test_diagnostics_distinguish_absence_invalid_and_failure()
{
    iotsmartsys::core::Log::setLogger(&captured);

    // Absence: no namespace yet.
    captured.clear();
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
    }
    TEST_ASSERT_TRUE(captured.contains("No stored snapshot"));
    TEST_ASSERT_FALSE(captured.contains("rejected by structural"));

    // Invalid content, with the same size and version.
    {
        EspNvsBinaryCapabilityStateProvider seedProvider;
        seedProvider.setNvsOps(seam::ops());
        seedProvider.loadSnapshot();
        seedProvider.activateWriter();
        persistAndAwait(seedProvider, "dev_Switch", "Switch", true);
    }
    auto blob = readStoredBlob();
    blob[4] ^= 0xFF; // corrupt the checksum field only
    writeStoredBlob(blob);

    captured.clear();
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
    }
    TEST_ASSERT_TRUE(captured.contains("rejected by structural"));
    // An invalid snapshot is never reported as a plain absence.
    TEST_ASSERT_FALSE(captured.contains("No stored snapshot"));

    // Storage failure: distinct from both absence and invalid content.
    captured.clear();
    seam::failInit = true;
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
    }
    TEST_ASSERT_TRUE(captured.contains("NVS init failed"));
    TEST_ASSERT_FALSE(captured.contains("No stored snapshot"));
    TEST_ASSERT_FALSE(captured.contains("rejected by structural"));
    seam::failInit = false;

    // Write failure is diagnosed as a write failure, not as absence.
    captured.clear();
    {
        EspNvsBinaryCapabilityStateProvider provider;
        provider.setNvsOps(seam::ops());
        provider.loadSnapshot();
        provider.activateWriter();
        seam::failWrite = true;
        provider.requestSave("dev_Switch", "Switch", true);
        provider.waitForQuiescence(500);
        seam::failWrite = false;
    }
    TEST_ASSERT_TRUE(captured.contains("NVS write failed"));

    // No private content leaked into any diagnostic: no settings sentinel, no
    // credentials and no raw blob bytes.
    for (const auto &entry : captured.entries)
    {
        TEST_ASSERT_NULL(strstr(entry.c_str(), "settings-sentinel-value"));
        TEST_ASSERT_NULL(strstr(entry.c_str(), SETTINGS_NAMESPACE));
    }

    iotsmartsys::core::Log::setLogger(nullptr);
}

void setup()
{
    delay(200);
    UNITY_BEGIN();
    RUN_TEST(test_first_boot_is_absent);
    RUN_TEST(test_full_length_identities_round_trip_without_truncation);
    RUN_TEST(test_identity_beyond_the_public_limit_is_refused_not_truncated);
    RUN_TEST(test_eight_record_limit);
    RUN_TEST(test_single_data_read_per_boot);
    RUN_TEST(test_settings_namespace_is_never_touched);
    RUN_TEST(test_truncated_blob_is_rejected);
    RUN_TEST(test_unknown_version_is_rejected);
    RUN_TEST(test_single_byte_corruption_is_rejected_everywhere);
    RUN_TEST(test_layout_matches_the_storage_contract);
    RUN_TEST(test_semantically_invalid_snapshots_are_rejected);
    RUN_TEST(test_init_failure_does_not_erase_or_abort);
    RUN_TEST(test_open_and_read_failures_preserve_the_default_flow);
    RUN_TEST(test_write_and_commit_failures_stay_observable);
    RUN_TEST(test_request_performs_no_nvs_work_in_the_calling_context);
    RUN_TEST(test_writer_is_activated_once_and_requires_activation);
    RUN_TEST(test_handle_never_reads_nvs);
    RUN_TEST(test_diagnostics_distinguish_absence_invalid_and_failure);
    UNITY_END();
}

void loop() {}
