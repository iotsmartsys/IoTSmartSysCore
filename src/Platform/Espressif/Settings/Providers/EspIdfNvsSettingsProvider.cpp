// Platform/Espressif/Settings/EspIdfNvsSettingsProvider.cpp
#include "EspIdfNvsSettingsProvider.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"

#include <cstdio>
#include "esp_system.h"
#include "esp_mac.h"
#include <cstring>
#include <algorithm>

namespace iotsmartsys::platform::espressif
{
    static const char *clientIdPrefix()
    {
#if defined(CONFIG_IDF_TARGET_ESP32S3)
        return "esp32s3";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
        return "esp32c3";
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
        return "esp32c2";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
        return "esp32c6";
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
        return "esp32h2";
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
        return "esp32s2";
#elif defined(CONFIG_IDF_TARGET_ESP32)
        return "esp32";
#else
        return "esp";
#endif
    }

    // Storage estável para Settings::clientId (const char*) sem heap.
    // Se você tiver vários devices/tasks chamando load(), isso ainda é ok se for só “um clientId por firmware”.
    static char s_clientId[32] = {0};

    static void ensureClientId(iotsmartsys::core::settings::Settings &dst)
    {
        // Se já veio preenchido, não sobrescreve.
        // if (dst.clientId != nullptr && dst.clientId[0] != '\0')
        //     return;

        uint8_t mac[6] = {0};
        esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);
        if (err != ESP_OK)
        {
            // fallback: base mac
            (void)esp_base_mac_addr_get(mac);
        }

        // últimos 3 bytes => 6 dígitos hex
        std::snprintf(s_clientId, sizeof(s_clientId), "%s-%02X%02X%02X",
                      clientIdPrefix(), mac[3], mac[4], mac[5]);

        dst.clientId = s_clientId;
    }

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

    void EspIdfNvsSettingsProvider::copyStrIfNotEmpty(char *dst, std::size_t dstSize, const std::string &src)
    {
        if (!src.empty())
            copyStr(dst, dstSize, src);
    }

    std::string EspIdfNvsSettingsProvider::toString(const char *src)
    {
        return (src && *src) ? std::string(src) : std::string();
    }

    void EspIdfNvsSettingsProvider::toStored(const core::settings::Settings &src, StoredSettings &dst, const StoredSettings *existing)
    {

        auto &logger = *iotsmartsys::core::ServiceProvider::instance().logger();
        if (existing)
        {
            dst = *existing;
        }
        else
        {
            std::memset(&dst, 0, sizeof(dst));
        }

        dst.version = STORAGE_VERSION;
        dst.in_config_mode = src.in_config_mode ? 1 : 0;
        dst.logLevel = static_cast<int>(src.logLevel);

        auto applyMqttConfig = [&](const core::settings::MqttConfig &srcCfg, StoredMqttConfig &dstCfg)
        {
            if (!srcCfg.host.empty())
            {
                copyStr(dstCfg.host, sizeof(dstCfg.host), srcCfg.host);
                dstCfg.port = srcCfg.port;
                copyStrIfNotEmpty(dstCfg.user, sizeof(dstCfg.user), srcCfg.user);
                copyStrIfNotEmpty(dstCfg.password, sizeof(dstCfg.password), srcCfg.password);
                copyStrIfNotEmpty(dstCfg.protocol, sizeof(dstCfg.protocol), srcCfg.protocol);
                dstCfg.ttl = srcCfg.ttl;
            }
        };

        applyMqttConfig(src.mqtt.primary, dst.mqtt.primary);
        applyMqttConfig(src.mqtt.secondary, dst.mqtt.secondary);

        // topics
        copyStrIfNotEmpty(dst.mqtt.announce_topic, sizeof(dst.mqtt.announce_topic), src.mqtt.announce_topic);
        copyStrIfNotEmpty(dst.mqtt.command_topic, sizeof(dst.mqtt.command_topic), src.mqtt.command_topic);
        copyStrIfNotEmpty(dst.mqtt.notify_topic, sizeof(dst.mqtt.notify_topic), src.mqtt.notify_topic);

        // firmware
        if (src.firmware.isValid())
        {
            copyStr(dst.firmware.url, sizeof(dst.firmware.url), src.firmware.url);
            copyStr(dst.firmware.manifest, sizeof(dst.firmware.manifest), src.firmware.manifest);
            dst.firmware.verify_sha256 = src.firmware.verify_sha256 ? 1 : 0;
        }

        copyStr(dst.firmware.update, sizeof(dst.firmware.update), src.firmware.update);

        // wifi (only apply when we have both fields)
        if (!src.wifi.ssid.empty() && !src.wifi.password.empty())
        {
            copyStr(dst.wifi.ssid, sizeof(dst.wifi.ssid), src.wifi.ssid);
            copyStr(dst.wifi.password, sizeof(dst.wifi.password), src.wifi.password);
        }

        // api
        if (src.api.isValid())
        {
            copyStr(dst.api.url, sizeof(dst.api.url), src.api.url);
            copyStr(dst.api.key, sizeof(dst.api.key), src.api.key);
            copyStr(dst.api.basic_auth, sizeof(dst.api.basic_auth), src.api.basic_auth);
        }
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
        dst.firmware.update = toString(src.firmware.update);
        dst.firmware.update = src.firmware.update;

        // wifi
        dst.wifi.ssid = toString(src.wifi.ssid);
        dst.wifi.password = toString(src.wifi.password);

        // api
        dst.api.url = toString(src.api.url);
        dst.api.key = toString(src.api.key);
        dst.api.basic_auth = toString(src.api.basic_auth);
        switch (src.logLevel)
        {
        case 0:
            dst.logLevel = iotsmartsys::core::LogLevel::Error;
            break;
        case 1:
            dst.logLevel = iotsmartsys::core::LogLevel::Warn;
            break;
        case 2:
            dst.logLevel = iotsmartsys::core::LogLevel::Info;
            break;
        case 3:
            dst.logLevel = iotsmartsys::core::LogLevel::Debug;
            break;
        case 4:
            dst.logLevel = iotsmartsys::core::LogLevel::Trace;
            break;
        default:
            dst.logLevel = iotsmartsys::core::LogLevel::Error;
            break;
        }
        ensureClientId(dst);
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
        _logger->info("EspIdfNvsSettingsProvider", "port mqtt: %d", settings.mqtt.primary.port);
        _logger->info("EspIdfNvsSettingsProvider", "firmware update method: %s", settings.firmware.update.c_str());

        _logger->info("EspIdfNvsSettingsProvider", "save() called");
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
        StoredSettings *existing = new StoredSettings();
        bool hasExisting = false;
        {
            size_t required = sizeof(StoredSettings);
            esp_err_t rerr = nvs_get_blob(h, NVS_KEY, existing, &required);
            if (rerr == ESP_OK && required == sizeof(StoredSettings) && existing->version == STORAGE_VERSION)
            {
                hasExisting = true;
            }
        }

        StoredSettings *stored = new StoredSettings();
        toStored(settings, *stored, hasExisting ? existing : nullptr);

        err = nvs_set_blob(h, NVS_KEY, stored, sizeof(*stored));
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs set_blob failed: %d", (int)err);
            delete existing;
            delete stored;
            nvs_close(h);
            return map_esp_err(err);
        }
        _logger->info("EspIdfNvsSettingsProvider", "nvs set_blob succeeded, committing...");
        err = nvs_commit(h);
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs commit failed: %d", (int)err);
        }
        else
        {
            _logger->info("EspIdfNvsSettingsProvider", "nvs commit succeeded");
        }
        delete existing;
        delete stored;
        nvs_close(h);
        return map_esp_err(err);
    }

    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::saveWiFiOnly(const core::settings::WifiConfig &wifi)
    {
        _logger->debug("EspIdfNvsSettingsProvider", "saveWiFiOnly() called");
        if (wifi.ssid.empty() || wifi.password.empty())
        {
            _logger->warn("EspIdfNvsSettingsProvider", "saveWiFiOnly: ignoring empty WiFi config (ssid/password not provided)");
            return StateResult::InvalidArg;
        }

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
        // so we can persist WiFi-only fields on first boot. Use heap allocation to avoid
        // exhausting the FreeRTOS task stack.
        StoredSettings *stored = new StoredSettings();
        size_t required = sizeof(StoredSettings);
        err = nvs_get_blob(h, NVS_KEY, stored, &required);
        if (err != ESP_OK || required != sizeof(StoredSettings))
        {
            // No existing blob or invalid size: create a new default-stored object.
            _logger->debug("EspIdfNvsSettingsProvider", "nvs get_blob not found or invalid size: %d - creating new stored blob", (int)err);
            std::memset(stored, 0, sizeof(*stored));
            stored->version = STORAGE_VERSION;
            // continue — we'll set wifi fields below and persist the new blob
        }

        // Update only WiFi settings
        copyStr(stored->wifi.ssid, sizeof(stored->wifi.ssid), wifi.ssid);
        _logger->debug("EspIdfNvsSettingsProvider", "Saving WiFi settings to NVS: SSID='%s'", wifi.ssid.c_str());
        // Never log passwords; log only length for diagnostics.
        _logger->debug("EspIdfNvsSettingsProvider", "Saving WiFi password_len=%u", (unsigned)wifi.password.size());

        copyStr(stored->wifi.password, sizeof(stored->wifi.password), wifi.password);

        // Debug: show what we'll write for WiFi (without password content)
        _logger->debug("EspIdfNvsSettingsProvider", "About to write stored.wifi: SSID='%s' password_len=%u", stored->wifi.ssid, (unsigned)std::strlen(stored->wifi.password));

        // Save back
        err = nvs_set_blob(h, NVS_KEY, stored, sizeof(*stored));
        if (err != ESP_OK)
        {
            _logger->error("EspIdfNvsSettingsProvider", "nvs set_blob failed: %d", (int)err);
            delete stored;
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
                StoredSettings *verify = new StoredSettings();
                size_t vreq = sizeof(*verify);
                rerr = nvs_get_blob(hr, NVS_KEY, verify, &vreq);
                if (rerr == ESP_OK && vreq == sizeof(*verify))
                {
                    _logger->debug("EspIdfNvsSettingsProvider", "Readback stored.wifi: SSID='%s' password_len=%u", verify->wifi.ssid, (unsigned)std::strlen(verify->wifi.password));
                }
                else
                {
                    _logger->warn("EspIdfNvsSettingsProvider", "Readback failed: err=%d vreq=%u", (int)rerr, (unsigned)vreq);
                }
                delete verify;
                nvs_close(hr);
            }
            else
            {
                _logger->warn("EspIdfNvsSettingsProvider", "Readback nvs_open failed: %d", (int)rerr);
            }
        }

        delete stored;
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
