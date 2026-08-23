// Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.h
#pragma once

#ifdef ESP32

#include "Contracts/Providers/IBinaryCapabilityStateProvider.h"
#include "Contracts/Logging/Log.h"

extern "C"
{
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
}

namespace iotsmartsys::platform::espressif
{
    // Espressif/NVS implementation of the binary-capability-state persistence
    // boundary. It uses its own NVS namespace, never touching the settings blob,
    // and keeps a compact versioned snapshot capped at 8 active records (the
    // runtime's capability limit).
    //
    // BCS-007: loadSnapshot() performs the single NVS data read for the boot and
    // tryGet() only ever consults the in-memory cache.
    // BCS-029/BCS-DEC-004: requestSave() returns without executing or awaiting
    // NVS; a single asynchronous worker serialises write and commit, keeping at
    // most one consolidated pending entry per identity.
    // BCS-027: no path in this provider erases the global NVS partition or uses
    // ESP_ERROR_CHECK/abort/restart to recover.
    class EspNvsBinaryCapabilityStateProvider final : public core::providers::IBinaryCapabilityStateProvider
    {
    public:
        EspNvsBinaryCapabilityStateProvider();
        ~EspNvsBinaryCapabilityStateProvider() override;

        core::common::StateResult loadSnapshot() override;
        core::common::StateResult activateWriter() override;
        bool tryGet(const char *capability_name, const char *type, bool &outIsOn) const override;
        core::common::StateResult requestSave(const char *capability_name, const char *type, bool isOn) override;
        core::providers::BinaryStateWriterStatus writerStatus() const override;

        // Test/reboot seam: waits until no work is pending and no write/commit
        // is in flight, or until the timeout expires. Quiescence alone is not
        // success; callers still read writerStatus() for the terminal result.
        bool waitForQuiescence(std::uint32_t timeoutMs);

        // NVS seam (spec 8.2): lets a test fail init, open, read, write and
        // commit individually, count operations per kind and prove no global
        // erase is ever emitted. It returns the very same codes as the real API
        // and preserves the property that only a successful commit survives a
        // reboot. Defaults to the real nvs_* calls.
        struct NvsOps
        {
            esp_err_t (*flashInit)();
            esp_err_t (*open)(const char *ns, nvs_open_mode_t mode, nvs_handle_t *out);
            esp_err_t (*getBlob)(nvs_handle_t handle, const char *key, void *out, std::size_t *length);
            esp_err_t (*setBlob)(nvs_handle_t handle, const char *key, const void *value, std::size_t length);
            esp_err_t (*commit)(nvs_handle_t handle);
            void (*close)(nvs_handle_t handle);
        };

        static const NvsOps &defaultNvsOps();
        void setNvsOps(const NvsOps &ops) { _nvs = ops; }

        static constexpr std::size_t MAX_RECORDS = 8;
        // BCS-002/BCS-DEC-005: room for the full public maxima (63/31 bytes)
        // plus their null terminators, so the storage is never more restrictive
        // than the identity the public API accepts.
        static constexpr std::size_t NAME_LEN = 64;
        static constexpr std::size_t TYPE_LEN = 32;

    private:
        static constexpr const char *NVS_NAMESPACE = "iotbcs";
        static constexpr const char *NVS_KEY = "state";
        static constexpr std::uint32_t STORAGE_VERSION = 2;
        static constexpr std::uint32_t WRITER_STACK_BYTES = 3072;
        static constexpr UBaseType_t WRITER_PRIORITY = 2;

        struct StoredRecord
        {
            char capability_name[NAME_LEN];
            char type[TYPE_LEN];
            std::uint8_t isOn;
            std::uint8_t used;
        };

        struct StoredSnapshot
        {
            std::uint32_t version;
            std::uint32_t checksum;
            StoredRecord records[MAX_RECORDS];
        };

        // Snapshot whose commit completed successfully: the one that survives a
        // reboot and the one tryGet() answers from.
        StoredSnapshot _committed{};
        // Snapshot the capabilities want persisted; each identity keeps only its
        // most recent stable value (BCS-029).
        StoredSnapshot _desired{};
        bool _cacheLoaded{false};

        NvsOps _nvs{defaultNvsOps()};
        SemaphoreHandle_t _mutex{nullptr};
        TaskHandle_t _writerTask{nullptr};
        bool _writerAvailable{false};
        volatile bool _writerStop{false};
        bool _writeInProgress{false};
        std::uint32_t _writes{0};
        std::uint32_t _commits{0};
        std::uint32_t _failures{0};
        core::common::StateResult _lastError{core::common::StateResult::Ok};

        static void writerTaskEntry(void *arg);
        void writerLoop();
        // Performs one write+commit of the given snapshot. Never erases the
        // partition and never aborts (BCS-027).
        core::common::StateResult writeAndCommit(const StoredSnapshot &snapshot);

        static int findRecordIndex(const StoredSnapshot &snapshot, const char *capability_name, const char *type);
        static std::uint8_t pendingCount(const StoredSnapshot &desired, const StoredSnapshot &committed);
        // BCS-006/BCS-012: structural, semantic and integrity validation of a
        // loaded snapshot. Size and version alone never prove validity.
        static bool validateSnapshot(const StoredSnapshot &snapshot);
        static bool fieldIsTerminated(const char *field, std::size_t capacity);
        // Returns false (no silent truncation) when src does not fit dstSize.
        static bool copyField(char *dst, std::size_t dstSize, const char *src);
        // FNV-1a over the header and every record slot, so any single-byte
        // mutation of an active record's region is detected.
        static std::uint32_t computeChecksum(const StoredSnapshot &snapshot);

        bool lock() const;
        void unlock() const;
    };
} // namespace iotsmartsys::platform::espressif

#endif // ESP32
