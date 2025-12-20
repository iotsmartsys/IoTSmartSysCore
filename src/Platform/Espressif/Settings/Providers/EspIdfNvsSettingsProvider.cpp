// Platform/Espressif/Settings/EspIdfNvsSettingsProvider.cpp
#include "EspIdfNvsSettingsProvider.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"

#include <cstring>
#include <algorithm>

namespace iotsmartsys::platform::espressif
{
    using namespace iotsmartsys::core;
    using core::common::StateResult;

    static StateResult map_esp_err(esp_err_t e)
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

    EspIdfNvsSettingsProvider::EspIdfNvsSettingsProvider()
        : _logger(iotsmartsys::core::ServiceProvider::instance().logger())
    {
    }

    esp_err_t EspIdfNvsSettingsProvider::ensureNvsInit()
    {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            // NVS inválido/cheio/versão mudou
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        return err;
    }

    void EspIdfNvsSettingsProvider::copyStr(char *dst, std::size_t dstSize, const std::string &src)
    {
        if (!dst || dstSize == 0)
            return;
        // copia truncando e garantindo '\0'
        const std::size_t n = std::min(dstSize - 1, src.size());
        std::memcpy(dst, src.data(), n);
        dst[n] = '\0';
    }

    std::string EspIdfNvsSettingsProvider::toString(const char *src)
    {
        return (src && *src) ? std::string(src) : std::string();
    }

    static std::uint8_t toUpdateU8(core::settings::FirmwareUpdateMethod m)
    {
        using core::settings::FirmwareUpdateMethod;
        switch (m)
        {
        case FirmwareUpdateMethod::NONE:
            return 0;
        case FirmwareUpdateMethod::OTA:
            return 1;
        case FirmwareUpdateMethod::AUTO:
            return 2;
        default:
            return 0;
        }
    }

    static core::settings::FirmwareUpdateMethod fromUpdateU8(std::uint8_t v)
    {
        using core::settings::FirmwareUpdateMethod;
        switch (v)
        {
        case 0:
            return FirmwareUpdateMethod::NONE;
        case 1:
            return FirmwareUpdateMethod::OTA;
        case 2:
            return FirmwareUpdateMethod::AUTO;
        default:
            return FirmwareUpdateMethod::NONE;
        }
    }

    void EspIdfNvsSettingsProvider::toStored(const core::settings::Settings &src, StoredSettings &dst)
    {
        std::memset(&dst, 0, sizeof(dst));
        dst.version = STORAGE_VERSION;
        dst.in_config_mode = src.in_config_mode ? 1 : 0;

        // MQTT primary
        copyStr(dst.mqtt.primary.host, sizeof(dst.mqtt.primary.host), src.mqtt.primary.host);
        dst.mqtt.primary.port = src.mqtt.primary.port;
        copyStr(dst.mqtt.primary.user, sizeof(dst.mqtt.primary.user), src.mqtt.primary.user);
        copyStr(dst.mqtt.primary.password, sizeof(dst.mqtt.primary.password), src.mqtt.primary.password);
        copyStr(dst.mqtt.primary.protocol, sizeof(dst.mqtt.primary.protocol), src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = src.mqtt.primary.ttl;

        // MQTT secondary
        copyStr(dst.mqtt.secondary.host, sizeof(dst.mqtt.secondary.host), src.mqtt.secondary.host);
        dst.mqtt.secondary.port = src.mqtt.secondary.port;
        copyStr(dst.mqtt.secondary.user, sizeof(dst.mqtt.secondary.user), src.mqtt.secondary.user);
        copyStr(dst.mqtt.secondary.password, sizeof(dst.mqtt.secondary.password), src.mqtt.secondary.password);
        copyStr(dst.mqtt.secondary.protocol, sizeof(dst.mqtt.secondary.protocol), src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = src.mqtt.secondary.ttl;

        // topics
        copyStr(dst.mqtt.announce_topic, sizeof(dst.mqtt.announce_topic), src.mqtt.announce_topic);
        copyStr(dst.mqtt.command_topic, sizeof(dst.mqtt.command_topic), src.mqtt.command_topic);
        copyStr(dst.mqtt.notify_topic, sizeof(dst.mqtt.notify_topic), src.mqtt.notify_topic);

        // firmware
        copyStr(dst.firmware.url, sizeof(dst.firmware.url), src.firmware.url);
        copyStr(dst.firmware.manifest, sizeof(dst.firmware.manifest), src.firmware.manifest);
        dst.firmware.verify_sha256 = src.firmware.verify_sha256 ? 1 : 0;
        dst.firmware.update = toUpdateU8(src.firmware.update);

        // wifi
        copyStr(dst.wifi.ssid, sizeof(dst.wifi.ssid), src.wifi.ssid);
        copyStr(dst.wifi.password, sizeof(dst.wifi.password), src.wifi.password);

        // api
        copyStr(dst.api.key, sizeof(dst.api.key), src.api.key);
        copyStr(dst.api.basic_auth, sizeof(dst.api.basic_auth), src.api.basic_auth);
    }

    void EspIdfNvsSettingsProvider::fromStored(const StoredSettings &src, core::settings::Settings &dst)
    {
        auto &logger = *iotsmartsys::core::ServiceProvider::instance().logger();
        dst.in_config_mode = (src.in_config_mode != 0);

        // MQTT primary
        dst.mqtt.primary.host = toString(src.mqtt.primary.host);
        dst.mqtt.primary.port = static_cast<int>(src.mqtt.primary.port);
        dst.mqtt.primary.user = toString(src.mqtt.primary.user);
        dst.mqtt.primary.password = toString(src.mqtt.primary.password);
        dst.mqtt.primary.protocol = toString(src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = static_cast<int>(src.mqtt.primary.ttl);

        // MQTT secondary
        dst.mqtt.secondary.host = toString(src.mqtt.secondary.host);
        dst.mqtt.secondary.port = static_cast<int>(src.mqtt.secondary.port);
        dst.mqtt.secondary.user = toString(src.mqtt.secondary.user);
        dst.mqtt.secondary.password = toString(src.mqtt.secondary.password);
        dst.mqtt.secondary.protocol = toString(src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = static_cast<int>(src.mqtt.secondary.ttl);

        // topics
        dst.mqtt.announce_topic = toString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toString(src.mqtt.notify_topic);

        // firmware
        dst.firmware.url = toString(src.firmware.url);
        dst.firmware.manifest = toString(src.firmware.manifest);
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = fromUpdateU8(src.firmware.update);

        // wifi
        dst.wifi.ssid = toString(src.wifi.ssid);
        logger.debug("EspIdfNvsSettingsProvider", "Loading WiFi settings from NVS: SSID='%s'", dst.wifi.ssid.c_str());
        dst.wifi.password = toString(src.wifi.password);
        // Never log passwords. If you need diagnostics, log only length.
        logger.debug("EspIdfNvsSettingsProvider", "Loading WiFi settings from NVS: password_len=%u", (unsigned)dst.wifi.password.size());

        // api
        dst.api.key = toString(src.api.key);
        dst.api.basic_auth = toString(src.api.basic_auth);
    }

    bool EspIdfNvsSettingsProvider::exists()
    {
        if (ensureNvsInit() != ESP_OK)
            return false;

        nvs_handle_t h;
        if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
            return false;

        size_t required = 0;
        esp_err_t err = nvs_get_blob(h, NVS_KEY, nullptr, &required);
        nvs_close(h);

        return (err == ESP_OK && required == sizeof(StoredSettings));
    }
    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::load(core::settings::Settings &out)
    {
        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
            return map_esp_err(err);

        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
        if (err != ESP_OK)
            return map_esp_err(err);

        size_t required = 0;
        err = nvs_get_blob(h, NVS_KEY, nullptr, &required);
        if (err != ESP_OK)
        {
            nvs_close(h);
            return map_esp_err(err); // ESP_ERR_NVS_NOT_FOUND etc.
        }

        if (required != sizeof(StoredSettings))
        {
            nvs_close(h);
            return iotsmartsys::core::common::StateResult::StorageReadFail;
        }

        StoredSettings stored{};
        err = nvs_get_blob(h, NVS_KEY, &stored, &required);
        nvs_close(h);
        if (err != ESP_OK)
            return map_esp_err(err);

        if (stored.version != STORAGE_VERSION)
            return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

        fromStored(stored, out);
        return iotsmartsys::core::common::StateResult::Ok;
    }

    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::save(const core::settings::Settings &settings)
    {

        _logger->debug("EspIdfNvsSettingsProvider", "save() called");
        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs init failed: %d", (int)err);
            return map_esp_err(err);
        }

        // Open NVS first so we can optionally preserve some fields (e.g., WiFi)
        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs open failed: %d", (int)err);
            return map_esp_err(err);
        }

        // Read current stored blob (if any). We'll preserve WiFi if the incoming settings doesn't include it.
        StoredSettings existing{};
        bool hasExisting = false;
        {
            size_t required = sizeof(StoredSettings);
            esp_err_t rerr = nvs_get_blob(h, NVS_KEY, &existing, &required);
            if (rerr == ESP_OK && required == sizeof(StoredSettings) && existing.version == STORAGE_VERSION)
            {
                hasExisting = true;
            }
        }

        StoredSettings stored{};
        toStored(settings, stored);

        // Preserve WiFi if caller did not provide it (common when settings come from API without WiFi fields)
        const bool incomingWifiEmpty = (stored.wifi.ssid[0] == '\0' && stored.wifi.password[0] == '\0');
        if (hasExisting && incomingWifiEmpty)
        {
            std::memcpy(&stored.wifi, &existing.wifi, sizeof(stored.wifi));
            _logger->debug("EspIdfNvsSettingsProvider", "Preserving WiFi from existing NVS blob (incoming settings had empty WiFi)");
        }

        err = nvs_set_blob(h, NVS_KEY, &stored, sizeof(stored));
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs set_blob failed: %d", (int)err);
            nvs_close(h);
            return map_esp_err(err);
        }

        err = nvs_commit(h);
        nvs_close(h);
        return map_esp_err(err);
    }

    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::saveWiFiOnly(const core::settings::WifiConfig &wifi)
    {
        _logger->debug("EspIdfNvsSettingsProvider", "saveWiFiOnly() called");
        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs init failed: %d", (int)err);
            return map_esp_err(err);
        }

        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs open failed: %d", (int)err);
            return map_esp_err(err);
        }

        // Load existing settings (if present). If not present, create a fresh StoredSettings
        // so we can persist WiFi-only fields on first boot.
        StoredSettings stored{};
        size_t required = sizeof(StoredSettings);
        err = nvs_get_blob(h, NVS_KEY, &stored, &required);
        if (err != ESP_OK || required != sizeof(StoredSettings))
        {
            // No existing blob or invalid size: create a new default-stored object.
            _logger->debug("EspIdfNvsSettingsProvider", "nvs get_blob not found or invalid size: %d - creating new stored blob", (int)err);
            std::memset(&stored, 0, sizeof(stored));
            stored.version = STORAGE_VERSION;
            // continue — we'll set wifi fields below and persist the new blob
        }

        // Update only WiFi settings

        copyStr(stored.wifi.ssid, sizeof(stored.wifi.ssid), wifi.ssid);
        _logger->debug("EspIdfNvsSettingsProvider", "Saving WiFi settings to NVS: SSID='%s'", wifi.ssid.c_str());
        // Never log passwords. If you need diagnostics, log only length.
        _logger->debug("EspIdfNvsSettingsProvider", "Saving WiFi password_len=%u", (unsigned)wifi.password.size());

        copyStr(stored.wifi.password, sizeof(stored.wifi.password), wifi.password);

        // Debug: show what we'll write for WiFi (without password)
        _logger->debug("EspIdfNvsSettingsProvider", "About to write stored.wifi: SSID='%s' password_len=%u", stored.wifi.ssid, (unsigned)std::strlen(stored.wifi.password));

        // Save back
        err = nvs_set_blob(h, NVS_KEY, &stored, sizeof(stored));
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs set_blob failed: %d", (int)err);
            nvs_close(h);
            return map_esp_err(err);
        }

        err = nvs_commit(h);
        _logger->debug("EspIdfNvsSettingsProvider", "nvs commit result: %d", (int)err);

        // Read-back verification (diagnostic): re-open and read blob to ensure WiFi was persisted
        if (err == ESP_OK)
        {
            nvs_handle_t hr;
            esp_err_t rerr = nvs_open(NVS_NAMESPACE, NVS_READONLY, &hr);
            if (rerr == ESP_OK)
            {
                StoredSettings verify{};
                size_t vreq = sizeof(verify);
                rerr = nvs_get_blob(hr, NVS_KEY, &verify, &vreq);
                if (rerr == ESP_OK && vreq == sizeof(verify))
                {
                    _logger->debug("EspIdfNvsSettingsProvider", "Readback stored.wifi: SSID='%s' password_len=%u", verify.wifi.ssid, (unsigned)std::strlen(verify.wifi.password));
                }
                else
                {
                    _logger->warn("EspIdfNvsSettingsProvider", "Readback failed: err=%d vreq=%u", (int)rerr, (unsigned)vreq);
                }
                nvs_close(hr);
            }
            else
            {
                _logger->warn("EspIdfNvsSettingsProvider", "Readback nvs_open failed: %d", (int)rerr);
            }
        }

        nvs_close(h);
        return map_esp_err(err);
    }

    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::erase()
    {
        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
            return map_esp_err(err);

        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs open failed: %d", (int)err);
            return map_esp_err(err);
        }

        err = nvs_erase_key(h, NVS_KEY);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            _logger->warn("EspIdfNvsSettingsProvider", "nvs key not found: %d", (int)err);
            err = ESP_OK;
        }

        if (err == ESP_OK)
        {
            _logger->info("EspIdfNvsSettingsProvider", "nvs key erased successfully");
            err = nvs_commit(h);
        }
        nvs_close(h);
        return map_esp_err(err);
    }
} // namespace iotsmartsys::platform::espressif