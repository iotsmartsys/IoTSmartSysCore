// Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.cpp
#include "EspNvsBinaryCapabilityStateProvider.h"

#ifdef ESP32
#include <cstring>

namespace iotsmartsys::platform::espressif
{
    using iotsmartsys::core::common::StateResult;
    using iotsmartsys::core::providers::BinaryStateWriterStatus;

    namespace
    {
        StateResult mapEspErr(esp_err_t e)
        {
            switch (e)
            {
            case ESP_OK:
                return StateResult::Ok;
#ifdef ESP_ERR_NVS_NOT_FOUND
            case ESP_ERR_NVS_NOT_FOUND:
                return StateResult::NotFound;
#endif
            case ESP_ERR_NO_MEM:
                return StateResult::NoMem;
            case ESP_ERR_INVALID_ARG:
                return StateResult::InvalidArg;
            case ESP_ERR_INVALID_STATE:
                return StateResult::InvalidState;
            case ESP_ERR_TIMEOUT:
                return StateResult::Timeout;
            default:
                return StateResult::StorageReadFail;
            }
        }

        // Resolved at call time (not cached), so logging works regardless of
        // this provider's construction order relative to Log::setLogger().
        iotsmartsys::core::ILogger &logger()
        {
            return iotsmartsys::core::Log::get();
        }
    } // namespace

    const EspNvsBinaryCapabilityStateProvider::NvsOps &EspNvsBinaryCapabilityStateProvider::defaultNvsOps()
    {
        // BCS-027: the default seam deliberately offers no erase operation, so
        // no code path in this provider can emit a global NVS erase.
        static const NvsOps ops{
            []() -> esp_err_t
            { return nvs_flash_init(); },
            [](const char *ns, nvs_open_mode_t mode, nvs_handle_t *out) -> esp_err_t
            { return nvs_open(ns, mode, out); },
            [](nvs_handle_t handle, const char *key, void *out, std::size_t *length) -> esp_err_t
            { return nvs_get_blob(handle, key, out, length); },
            [](nvs_handle_t handle, const char *key, const void *value, std::size_t length) -> esp_err_t
            { return nvs_set_blob(handle, key, value, length); },
            [](nvs_handle_t handle) -> esp_err_t
            { return nvs_commit(handle); },
            [](nvs_handle_t handle)
            { nvs_close(handle); }};
        return ops;
    }

    EspNvsBinaryCapabilityStateProvider::EspNvsBinaryCapabilityStateProvider() = default;

    EspNvsBinaryCapabilityStateProvider::~EspNvsBinaryCapabilityStateProvider()
    {
        if (_writerTask)
        {
            _writerStop = true;
            xTaskNotifyGive(_writerTask);
        }
        if (_mutex)
        {
            vSemaphoreDelete(_mutex);
            _mutex = nullptr;
        }
    }

    bool EspNvsBinaryCapabilityStateProvider::lock() const
    {
        if (!_mutex)
        {
            // Before activateWriter() the provider is only touched by the
            // single-threaded bootstrap, so the absence of a mutex is not a
            // failure.
            return true;
        }
        return xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE;
    }

    void EspNvsBinaryCapabilityStateProvider::unlock() const
    {
        if (_mutex)
        {
            xSemaphoreGive(_mutex);
        }
    }

    bool EspNvsBinaryCapabilityStateProvider::copyField(char *dst, std::size_t dstSize, const char *src)
    {
        if (!dst || dstSize == 0)
            return false;
        const std::size_t len = src ? std::strlen(src) : 0;
        // BCS-002/5.2: reject identities that do not fit instead of truncating
        // them silently, which would collide by prefix.
        if (len >= dstSize)
            return false;
        if (src && len > 0)
        {
            std::memcpy(dst, src, len);
        }
        std::memset(dst + len, 0, dstSize - len);
        return true;
    }

    std::uint32_t EspNvsBinaryCapabilityStateProvider::computeChecksum(const StoredSnapshot &snapshot)
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

    bool EspNvsBinaryCapabilityStateProvider::fieldIsTerminated(const char *field, std::size_t capacity)
    {
        for (std::size_t i = 0; i < capacity; ++i)
        {
            if (field[i] == '\0')
                return true;
        }
        return false;
    }

    bool EspNvsBinaryCapabilityStateProvider::validateSnapshot(const StoredSnapshot &snapshot)
    {
        if (snapshot.version != STORAGE_VERSION)
            return false;

        std::size_t active = 0;
        for (std::size_t i = 0; i < MAX_RECORDS; ++i)
        {
            const auto &rec = snapshot.records[i];

            // Domain of the flags: anything outside {0,1} invalidates the blob.
            if (rec.used > 1 || rec.isOn > 1)
                return false;

            // Both fields must terminate inside their own storage, whether the
            // record is active or not, so no strcmp can ever run off the field.
            if (!fieldIsTerminated(rec.capability_name, NAME_LEN) ||
                !fieldIsTerminated(rec.type, TYPE_LEN))
                return false;

            if (rec.used == 0)
                continue;

            ++active;
            // An active record with an empty identity cannot be matched and
            // indicates a corrupt or partially written snapshot.
            if (rec.capability_name[0] == '\0' || rec.type[0] == '\0')
                return false;
        }

        if (active > MAX_RECORDS)
            return false;

        // Integrity last: it covers the header and every record slot, so size
        // and version matching is never sufficient on its own.
        return computeChecksum(snapshot) == snapshot.checksum;
    }

    int EspNvsBinaryCapabilityStateProvider::findRecordIndex(const StoredSnapshot &snapshot,
                                                            const char *capability_name,
                                                            const char *type)
    {
        for (std::size_t i = 0; i < MAX_RECORDS; ++i)
        {
            const auto &rec = snapshot.records[i];
            if (rec.used && std::strcmp(rec.capability_name, capability_name) == 0 &&
                std::strcmp(rec.type, type) == 0)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    std::uint8_t EspNvsBinaryCapabilityStateProvider::pendingCount(const StoredSnapshot &desired,
                                                                  const StoredSnapshot &committed)
    {
        std::uint8_t pending = 0;
        for (std::size_t i = 0; i < MAX_RECORDS; ++i)
        {
            const auto &d = desired.records[i];
            const auto &c = committed.records[i];
            if (d.used != c.used || d.isOn != c.isOn ||
                std::strncmp(d.capability_name, c.capability_name, NAME_LEN) != 0 ||
                std::strncmp(d.type, c.type, TYPE_LEN) != 0)
            {
                ++pending;
            }
        }
        return pending;
    }

    iotsmartsys::core::common::StateResult EspNvsBinaryCapabilityStateProvider::loadSnapshot()
    {
        std::memset(&_committed, 0, sizeof(_committed));
        _committed.version = STORAGE_VERSION;
        _desired = _committed;
        _cacheLoaded = true;

        // BCS-027: initialise NVS but never erase the partition to recover, and
        // never wrap the call in ESP_ERROR_CHECK. A failure here degrades the
        // binary domain only; settings and the runtime stay untouched.
        esp_err_t err = _nvs.flashInit();
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS init failed (rc=%d); binary domain unavailable, no erase performed.",
                           static_cast<int>(err));
            return mapEspErr(err);
        }

        nvs_handle_t h;
        err = _nvs.open(NVS_NAMESPACE, NVS_READONLY, &h);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            // No namespace yet (first boot / erased NVS) is a legitimate absence,
            // not a failure (BCS-011): the cache stays empty and defaults apply.
            logger().info("BinaryCapabilityState", "No stored snapshot (rc=%d); using defaults.", static_cast<int>(err));
            return StateResult::Ok;
        }
        if (err != ESP_OK)
        {
            // BCS-017/BCS-021: only NOT_FOUND means absence. An unavailable or
            // otherwise failing namespace remains an observable storage failure.
            logger().error("BinaryCapabilityState", "NVS namespace open failed on read (rc=%d).", static_cast<int>(err));
            return mapEspErr(err);
        }

        // Metadata-only query: it does not copy blob content and therefore is
        // not the single data read allowed by BCS-007.
        std::size_t required = 0;
        err = _nvs.getBlob(h, NVS_KEY, nullptr, &required);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            _nvs.close(h);
            logger().info("BinaryCapabilityState", "No stored snapshot key (rc=%d); using defaults.", static_cast<int>(err));
            return StateResult::Ok;
        }
        if (err != ESP_OK)
        {
            _nvs.close(h);
            // BCS-017/BCS-021: metadata read failures are storage failures and
            // must never be reported as a missing key.
            logger().error("BinaryCapabilityState", "Snapshot metadata read failed (rc=%d).", static_cast<int>(err));
            return mapEspErr(err);
        }

        if (required != sizeof(StoredSnapshot))
        {
            _nvs.close(h);
            logger().warn("BinaryCapabilityState", "Snapshot size mismatch (got %u, expected %u); treating as invalid.",
                          static_cast<unsigned>(required), static_cast<unsigned>(sizeof(StoredSnapshot)));
            return StateResult::StorageCorrupt;
        }

        StoredSnapshot loaded{};
        err = _nvs.getBlob(h, NVS_KEY, &loaded, &required);
        _nvs.close(h);
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "Snapshot read failed (rc=%d).", static_cast<int>(err));
            return mapEspErr(err);
        }

        if (loaded.version != STORAGE_VERSION)
        {
            logger().warn("BinaryCapabilityState", "Snapshot version mismatch (got %u, expected %u); treating as invalid.",
                          static_cast<unsigned>(loaded.version), static_cast<unsigned>(STORAGE_VERSION));
            return StateResult::StorageVersionMismatch;
        }

        // BCS-006/BCS-012: a record failing any structural, semantic or
        // integrity check invalidates the whole snapshot; none of its values is
        // applied, and nothing is compared with strcmp before this point.
        if (!validateSnapshot(loaded))
        {
            logger().warn("BinaryCapabilityState", "Snapshot rejected by structural/semantic/integrity validation; treating as invalid.");
            return StateResult::StorageCorrupt;
        }

        _committed = loaded;
        _desired = loaded;
        logger().info("BinaryCapabilityState", "Snapshot loaded from NVS.");
        return StateResult::Ok;
    }

    iotsmartsys::core::common::StateResult EspNvsBinaryCapabilityStateProvider::activateWriter()
    {
        if (_writerAvailable)
        {
            // BCS-024/BCS-029: exactly one worker per boot.
            return StateResult::Ok;
        }

        if (!_mutex)
        {
            _mutex = xSemaphoreCreateMutex();
            if (!_mutex)
            {
                _lastError = StateResult::NoMem;
                logger().error("BinaryCapabilityState", "Writer mutex creation failed; writer unavailable (no synchronous fallback).");
                return StateResult::NoMem;
            }
        }

        const BaseType_t ok = xTaskCreate(&EspNvsBinaryCapabilityStateProvider::writerTaskEntry,
                                          "bcs_writer",
                                          WRITER_STACK_BYTES,
                                          this,
                                          WRITER_PRIORITY,
                                          &_writerTask);
        if (ok != pdPASS)
        {
            _writerTask = nullptr;
            _lastError = StateResult::NoMem;
            // BCS-029: a creation failure stays observable and never authorises
            // a synchronous write from the requesting context.
            logger().error("BinaryCapabilityState", "Writer task creation failed; writer unavailable (no synchronous fallback).");
            return StateResult::NoMem;
        }

        _writerAvailable = true;
        logger().info("BinaryCapabilityState", "Asynchronous snapshot writer activated.");
        return StateResult::Ok;
    }

    bool EspNvsBinaryCapabilityStateProvider::tryGet(const char *capability_name, const char *type, bool &outIsOn) const
    {
        if (!_cacheLoaded || !capability_name || !type)
            return false;

        if (!lock())
            return false;

        // BCS-018: answers from the last successfully committed snapshot, which
        // is exactly what survives a reboot.
        const int idx = findRecordIndex(_committed, capability_name, type);
        const bool found = idx >= 0;
        if (found)
        {
            outIsOn = (_committed.records[idx].isOn != 0);
        }
        unlock();
        return found;
    }

    iotsmartsys::core::common::StateResult EspNvsBinaryCapabilityStateProvider::requestSave(const char *capability_name,
                                                                                           const char *type,
                                                                                           bool isOn)
    {
        if (!capability_name || !type || !*capability_name || !*type)
            return StateResult::InvalidArg;

        // BCS-002: the storage is never more restrictive than the public
        // identity contract; anything that still does not fit is rejected, never
        // truncated.
        if (std::strlen(capability_name) >= NAME_LEN || std::strlen(type) >= TYPE_LEN)
        {
            logger().error("BinaryCapabilityState", "Identity too long for '%s'; rejecting instead of truncating.", capability_name);
            return StateResult::InvalidArg;
        }

        if (!_writerAvailable)
        {
            // No synchronous fallback: the caller keeps its confirmed hardware
            // and logical state, and the refusal stays observable.
            return StateResult::InvalidState;
        }

        if (!lock())
            return StateResult::Timeout;

        int idx = findRecordIndex(_desired, capability_name, type);
        if (idx < 0)
        {
            for (std::size_t i = 0; i < MAX_RECORDS; ++i)
            {
                if (!_desired.records[i].used)
                {
                    idx = static_cast<int>(i);
                    break;
                }
            }
        }

        if (idx < 0)
        {
            unlock();
            logger().error("BinaryCapabilityState", "No free slot to persist '%s' (limit %u reached).",
                           capability_name, static_cast<unsigned>(MAX_RECORDS));
            return StateResult::Overflow;
        }

        // 5.2/BCS-029: the identity keeps only its most recent stable value;
        // repeated transitions before the next write are consolidated in place,
        // with no queue growth and no per-transition allocation.
        _desired.version = STORAGE_VERSION;
        auto &rec = _desired.records[idx];
        if (!copyField(rec.capability_name, sizeof(rec.capability_name), capability_name) ||
            !copyField(rec.type, sizeof(rec.type), type))
        {
            unlock();
            return StateResult::InvalidArg;
        }
        rec.isOn = isOn ? 1 : 0;
        rec.used = 1;
        _desired.checksum = computeChecksum(_desired);
        unlock();

        // Returns immediately: no nvs_set_blob(), no nvs_commit(), no waiting.
        xTaskNotifyGive(_writerTask);
        return StateResult::Ok;
    }

    iotsmartsys::core::providers::BinaryStateWriterStatus EspNvsBinaryCapabilityStateProvider::writerStatus() const
    {
        BinaryStateWriterStatus status;
        if (!lock())
            return status;

        status.available = _writerAvailable;
        status.pending = pendingCount(_desired, _committed);
        status.inProgress = _writeInProgress;
        status.writes = _writes;
        status.commits = _commits;
        status.failures = _failures;
        status.lastError = _lastError;
        unlock();
        return status;
    }

    bool EspNvsBinaryCapabilityStateProvider::waitForQuiescence(std::uint32_t timeoutMs)
    {
        const TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeoutMs);
        while (true)
        {
            const auto status = writerStatus();
            if (status.pending == 0 && !status.inProgress)
                return true;
            if (static_cast<int32_t>(xTaskGetTickCount() - deadline) >= 0)
                return false;
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }

    void EspNvsBinaryCapabilityStateProvider::writerTaskEntry(void *arg)
    {
        auto *self = static_cast<EspNvsBinaryCapabilityStateProvider *>(arg);
        if (self)
        {
            self->writerLoop();
        }
        vTaskDelete(nullptr);
    }

    void EspNvsBinaryCapabilityStateProvider::writerLoop()
    {
        while (!_writerStop)
        {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            if (_writerStop)
                break;

            // Drain: consolidate whatever is pending, one write/commit at a
            // time, until desired and committed converge.
            while (!_writerStop)
            {
                StoredSnapshot attempt{};
                if (!lock())
                    break;
                if (pendingCount(_desired, _committed) == 0)
                {
                    unlock();
                    break;
                }
                attempt = _desired;
                _writeInProgress = true;
                unlock();

                const StateResult rc = writeAndCommit(attempt);

                if (!lock())
                    break;
                _writeInProgress = false;
                if (rc == StateResult::Ok)
                {
                    // BCS-018: only a successful commit updates the view of the
                    // last persisted snapshot. A transition that arrived during
                    // the operation stays pending because it diverges from what
                    // this commit actually confirmed.
                    _committed = attempt;
                    ++_commits;
                    _lastError = StateResult::Ok;
                    unlock();
                }
                else
                {
                    ++_failures;
                    _lastError = rc;
                    unlock();
                    // BCS-017/7: no continuous retry; the next attempt happens
                    // when another confirmed stable transition updates the
                    // identity and signals the writer again.
                    break;
                }
            }
        }

        _writerTask = nullptr;
    }

    iotsmartsys::core::common::StateResult EspNvsBinaryCapabilityStateProvider::writeAndCommit(const StoredSnapshot &snapshot)
    {
        // BCS-027: no erase, no ESP_ERROR_CHECK, no abort/restart in this path.
        nvs_handle_t h;
        esp_err_t err = _nvs.open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS open failed on write (rc=%d); state kept in memory.", static_cast<int>(err));
            return mapEspErr(err);
        }

        if (!lock())
        {
            _nvs.close(h);
            return StateResult::Timeout;
        }
        ++_writes;
        unlock();

        err = _nvs.setBlob(h, NVS_KEY, &snapshot, sizeof(snapshot));
        if (err != ESP_OK)
        {
            _nvs.close(h);
            logger().error("BinaryCapabilityState", "NVS write failed (rc=%d); confirmed hardware/logical state unaffected.",
                           static_cast<int>(err));
            return StateResult::StorageWriteFail;
        }

        err = _nvs.commit(h);
        _nvs.close(h);
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS commit failed (rc=%d); last successful commit remains the persisted snapshot.",
                           static_cast<int>(err));
            return StateResult::StorageWriteFail;
        }

        return StateResult::Ok;
    }
} // namespace iotsmartsys::platform::espressif

#endif // ESP32
