// Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.cpp
#include "EspNvsBinaryCapabilityStateProvider.h"

#ifdef ESP32
#include <cstring>

namespace iotsmartsys::platform::espressif
{
    using iotsmartsys::core::common::StateResult;

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

    EspNvsBinaryCapabilityStateProvider::EspNvsBinaryCapabilityStateProvider() = default;

    esp_err_t EspNvsBinaryCapabilityStateProvider::ensureNvsInit()
    {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        return err;
    }

    bool EspNvsBinaryCapabilityStateProvider::copyField(char *dst, std::size_t dstSize, const char *src)
    {
        if (!dst || dstSize == 0)
            return false;
        const std::size_t len = src ? std::strlen(src) : 0;
        // BCS-002/5.2: reject identities that do not fit instead of
        // truncating them silently, which would collide by prefix.
        if (len >= dstSize)
            return false;
        if (src)
        {
            std::memcpy(dst, src, len);
        }
        dst[len] = '\0';
        return true;
    }

    std::uint32_t EspNvsBinaryCapabilityStateProvider::computeChecksum(const StoredSnapshot &snapshot)
    {
        // FNV-1a 32-bit over the header (version) and every record slot
        // (used and unused), so any single-byte mutation in an active
        // record's region is detected regardless of its offset.
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

    int EspNvsBinaryCapabilityStateProvider::findRecordIndex(const char *capability_name, const char *type) const
    {
        for (std::size_t i = 0; i < MAX_RECORDS; ++i)
        {
            const auto &rec = _cache.records[i];
            if (rec.used && std::strcmp(rec.capability_name, capability_name) == 0 && std::strcmp(rec.type, type) == 0)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    iotsmartsys::core::common::StateResult EspNvsBinaryCapabilityStateProvider::loadSnapshot()
    {
        std::memset(&_cache, 0, sizeof(_cache));
        _cacheLoaded = true;

        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS init failed: %d", static_cast<int>(err));
            return mapEspErr(err);
        }

        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
        if (err != ESP_OK)
        {
            // No namespace yet (first boot / erased NVS) is a legitimate absence,
            // not a failure (BCS-011): the cache stays empty and defaults apply.
            logger().info("BinaryCapabilityState", "No stored snapshot (rc=%d); using defaults.", static_cast<int>(err));
            return StateResult::Ok;
        }

        std::size_t required = 0;
        err = nvs_get_blob(h, NVS_KEY, nullptr, &required);
        if (err != ESP_OK)
        {
            nvs_close(h);
            logger().info("BinaryCapabilityState", "No stored snapshot key (rc=%d); using defaults.", static_cast<int>(err));
            return StateResult::Ok;
        }

        if (required != sizeof(StoredSnapshot))
        {
            nvs_close(h);
            logger().warn("BinaryCapabilityState", "Snapshot size mismatch (got %u, expected %u); treating as absent.",
                          static_cast<unsigned>(required), static_cast<unsigned>(sizeof(StoredSnapshot)));
            return StateResult::StorageCorrupt;
        }

        StoredSnapshot loaded{};
        err = nvs_get_blob(h, NVS_KEY, &loaded, &required);
        nvs_close(h);
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "Snapshot read failed: %d", static_cast<int>(err));
            return mapEspErr(err);
        }

        if (loaded.version != STORAGE_VERSION)
        {
            logger().warn("BinaryCapabilityState", "Snapshot version mismatch (got %u, expected %u); treating as absent.",
                          static_cast<unsigned>(loaded.version), static_cast<unsigned>(STORAGE_VERSION));
            return StateResult::StorageVersionMismatch;
        }

        // BCS-006/BCS-012: size and version alone do not prove integrity;
        // reject any content whose checksum does not cover cleanly.
        if (computeChecksum(loaded) != loaded.checksum)
        {
            logger().warn("BinaryCapabilityState", "Snapshot checksum mismatch; treating as absent.");
            return StateResult::StorageCorrupt;
        }

        _cache = loaded;
        logger().info("BinaryCapabilityState", "Snapshot loaded from NVS.");
        return StateResult::Ok;
    }

    bool EspNvsBinaryCapabilityStateProvider::tryGet(const char *capability_name, const char *type, bool &outIsOn) const
    {
        if (!_cacheLoaded || !capability_name || !type)
            return false;

        const int idx = findRecordIndex(capability_name, type);
        if (idx < 0)
            return false;

        outIsOn = (_cache.records[idx].isOn != 0);
        return true;
    }

    iotsmartsys::core::common::StateResult EspNvsBinaryCapabilityStateProvider::save(const char *capability_name, const char *type, bool isOn)
    {
        if (!capability_name || !type || !*capability_name || !*type)
            return StateResult::InvalidArg;

        // BCS-002/5.2: reject identities that do not fit the internal buffer
        // instead of truncating them silently, which could collide by prefix.
        if (std::strlen(capability_name) >= NAME_LEN || std::strlen(type) >= TYPE_LEN)
        {
            logger().error("BinaryCapabilityState", "Identity too long for '%s'; rejecting instead of truncating.", capability_name);
            return StateResult::InvalidArg;
        }

        // 5.2: a write updates the in-memory snapshot first, then persists the
        // blob, then commits.
        int idx = findRecordIndex(capability_name, type);
        if (idx < 0)
        {
            for (std::size_t i = 0; i < MAX_RECORDS; ++i)
            {
                if (!_cache.records[i].used)
                {
                    idx = static_cast<int>(i);
                    break;
                }
            }
        }

        if (idx < 0)
        {
            logger().error("BinaryCapabilityState", "No free slot to persist '%s' (limit %u reached).",
                          capability_name, static_cast<unsigned>(MAX_RECORDS));
            return StateResult::Overflow;
        }

        _cache.version = STORAGE_VERSION;
        auto &rec = _cache.records[idx];
        copyField(rec.capability_name, sizeof(rec.capability_name), capability_name);
        copyField(rec.type, sizeof(rec.type), type);
        rec.isOn = isOn ? 1 : 0;
        rec.used = 1;
        _cache.checksum = computeChecksum(_cache);

        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS init failed on save: %d", static_cast<int>(err));
            return mapEspErr(err);
        }

        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS open failed on save: %d", static_cast<int>(err));
            return mapEspErr(err);
        }

        err = nvs_set_blob(h, NVS_KEY, &_cache, sizeof(_cache));
        if (err != ESP_OK)
        {
            nvs_close(h);
            logger().error("BinaryCapabilityState", "NVS set_blob failed on save: %d", static_cast<int>(err));
            return mapEspErr(err);
        }

        err = nvs_commit(h);
        nvs_close(h);
        if (err != ESP_OK)
        {
            logger().error("BinaryCapabilityState", "NVS commit failed on save: %d", static_cast<int>(err));
        }

        return mapEspErr(err);
    }
} // namespace iotsmartsys::platform::espressif

#endif // ESP32
