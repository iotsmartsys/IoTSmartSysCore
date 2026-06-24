// Platform/Espressif/Settings/EspIdfNvsSettingsProvider.cpp
#include "EspIdfNvsSettingsProvider.h"

#ifdef ESP32
#include "Config/BuildConfig.h"
#include "Config/WifiCredentials.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Espressif/Providers/DeviceIdentityProvider.h"

#include <cstdio>
#include "esp_system.h"
#include "esp_mac.h"
#include <cstring>
#include <algorithm>

namespace iotsmartsys::platform::espressif
{
    namespace
    {
        struct LegacyStoredMqttConfigV3
        {
            char host[128];
            std::int32_t port;
            char user[64];
            char password[64];
            char protocol[8];
            std::int32_t ttl;
        };

        struct LegacyStoredMqttSettingsV3
        {
            LegacyStoredMqttConfigV3 primary;
            LegacyStoredMqttConfigV3 secondary;
            char announce_topic[128];
            char command_topic[128];
            char notify_topic[128];
            char profile[16];
        };

        struct LegacyStoredFirmwareConfigV3
        {
            char url[128];
            char manifest[160];
            std::uint8_t verify_sha256;
            char update[8];
        };

        struct LegacyStoredWifiConfigV3
        {
            char ssid[64];
            char password[64];
        };

        struct LegacyStoredApiConfigV3
        {
            char url[128];
            char key[128];
            char basic_auth[160];
        };

        struct LegacyStoredSettingsV3
        {
            std::uint32_t version;
            std::uint8_t in_config_mode;
            std::int32_t logLevel;

            LegacyStoredMqttSettingsV3 mqtt;
            LegacyStoredFirmwareConfigV3 firmware;
            LegacyStoredWifiConfigV3 wifi;
            LegacyStoredApiConfigV3 api;
        };

        struct LegacyStoredMqttConfigV4
        {
            char host[128];
            std::int32_t port;
            char user[64];
            char password[64];
            char protocol[8];
            std::int32_t ttl;
        };

        struct LegacyStoredMqttSettingsV4
        {
            LegacyStoredMqttConfigV4 primary;
            LegacyStoredMqttConfigV4 secondary;
            char announce_topic[128];
            char command_topic[128];
            char notify_topic[128];
            char profile[16];
        };

        struct LegacyStoredFirmwareConfigV4
        {
            char url[128];
            char manifest[160];
            std::uint8_t verify_sha256;
            char update[8];
        };

        struct LegacyStoredWifiConfigV4
        {
            char ssid[64];
            char password[64];
        };

        struct LegacyStoredApiConfigV4
        {
            char url[128];
            char key[128];
            char basic_auth[160];
        };

        struct LegacyStoredSettingsV4
        {
            std::uint32_t version;
            std::uint8_t in_config_mode;
            std::int32_t logLevel;

            LegacyStoredMqttSettingsV4 mqtt;
            LegacyStoredFirmwareConfigV4 firmware;
            LegacyStoredWifiConfigV4 wifi;
            LegacyStoredApiConfigV4 api;
        };

        struct LegacyStoredMqttConfigV5
        {
            char host[128];
            std::int32_t port;
            char user[64];
            char password[64];
            char protocol[8];
            std::int32_t ttl;
        };

        struct LegacyStoredMqttSettingsV5
        {
            LegacyStoredMqttConfigV5 primary;
            LegacyStoredMqttConfigV5 secondary;
            LegacyStoredMqttConfigV5 tertiary;
            char announce_topic[128];
            char command_topic[128];
            char notify_topic[128];
            char profile[16];
        };

        struct LegacyStoredFirmwareConfigV5
        {
            char url[128];
            char manifest[160];
            std::uint8_t verify_sha256;
            char update[8];
        };

        struct LegacyStoredWifiConfigV5
        {
            char ssid[64];
            char password[64];
        };

        struct LegacyStoredApiConfigV5
        {
            char url[128];
            char key[128];
            char basic_auth[160];
        };

        struct LegacyStoredSettingsV5
        {
            std::uint32_t version;
            std::uint8_t in_config_mode;
            std::uint8_t device_registered;
            std::int32_t logLevel;

            LegacyStoredMqttSettingsV5 mqtt;
            LegacyStoredFirmwareConfigV5 firmware;
            LegacyStoredWifiConfigV5 wifi;
            LegacyStoredApiConfigV5 api;
        };

        struct LegacyStoredMqttConfigV6
        {
            char host[128];
            std::int32_t port;
            char user[64];
            char password[64];
            char protocol[8];
            std::int32_t ttl;
        };

        struct LegacyStoredMqttSettingsV6
        {
            LegacyStoredMqttConfigV6 primary;
            LegacyStoredMqttConfigV6 secondary;
            LegacyStoredMqttConfigV6 tertiary;
            char announce_topic[128];
            char command_topic[128];
            char notify_topic[128];
            char profile[16];
        };

        struct LegacyStoredFirmwareConfigV6
        {
            char url[128];
            char manifest[160];
            std::uint8_t verify_sha256;
            char update[8];
        };

        struct LegacyStoredWifiConfigV6
        {
            char primary_ssid[64];
            char primary_password[64];
            char secondary_ssid[64];
            char secondary_password[64];
            char tertiary_ssid[64];
            char tertiary_password[64];
            char profile[16];
            char ssid[64];
            char password[64];
        };

        struct LegacyStoredApiConfigV6
        {
            char url[128];
            char key[128];
            char basic_auth[160];
        };

        struct LegacyStoredSettingsV6
        {
            std::uint32_t version;
            std::uint8_t in_config_mode;
            std::uint8_t device_registered;
            std::int32_t logLevel;

            LegacyStoredMqttSettingsV6 mqtt;
            LegacyStoredFirmwareConfigV6 firmware;
            LegacyStoredWifiConfigV6 wifi;
            LegacyStoredApiConfigV6 api;
        };

        const char *compiledEnvironmentId()
        {
#ifdef IOTSMARTSYS_ENV_ID
            return IOTSMARTSYS_ENV_ID;
#else
            return "";
#endif
        }

        std::string resolveFirmwareManifest(std::string manifest)
        {
            static constexpr const char *kEnvIdPlaceholder = "{env_id}";
            const char *envId = compiledEnvironmentId();
            if (!envId || envId[0] == '\0')
            {
                return manifest;
            }

            std::size_t pos = 0;
            while ((pos = manifest.find(kEnvIdPlaceholder, pos)) != std::string::npos)
            {
                manifest.replace(pos, std::strlen(kEnvIdPlaceholder), envId);
                pos += std::strlen(envId);
            }

            return manifest;
        }

        void applyCompiledWifiOverride(core::settings::Settings &settings)
        {
            if (settings.wifi.isValid())
            {
                settings.wifi.syncSelectedLegacyFields();
                return;
            }

            if (iotsmartsys::config::hasWifiCredentials())
            {
                settings.wifi.ssid = iotsmartsys::config::kWifiCredentials[0].ssid;
                settings.wifi.password = iotsmartsys::config::kWifiCredentials[0].password;
                settings.wifi.primary.ssid = settings.wifi.ssid;
                settings.wifi.primary.password = settings.wifi.password;
                settings.wifi.profile = "primary";
                settings.in_config_mode = false;
                return;
            }

#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
            settings.wifi.ssid = WIFI_SSID;
            settings.wifi.password = WIFI_PASSWORD;
            settings.wifi.primary.ssid = settings.wifi.ssid;
            settings.wifi.primary.password = settings.wifi.password;
            settings.wifi.profile = "primary";
            settings.in_config_mode = false;
#endif
        }
    } // namespace

    // Storage estável para Settings::clientId (const char*) sem heap.
    // Se você tiver vários devices/tasks chamando load(), isso ainda é ok se for só “um clientId por firmware”.
    static char s_clientId[32] = {0};

    static void ensureClientId(iotsmartsys::core::settings::Settings &dst)
    {
        iotsmartsys::platform::espressif::providers::DeviceIdentityProvider identityProvider;
        const std::string deviceId = identityProvider.getDeviceID();
        const std::size_t n = std::min(sizeof(s_clientId) - 1, deviceId.size());
        std::memcpy(s_clientId, deviceId.data(), n);
        s_clientId[n] = '\0';

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

    static void fromLegacyStoredV3(const LegacyStoredSettingsV3 &src, core::settings::Settings &dst)
    {
        const auto toStdString = [](const char *value) -> std::string
        {
            return (value && *value) ? std::string(value) : std::string();
        };

        dst.in_config_mode = (src.in_config_mode != 0);
        dst.device_registered = false;

        dst.mqtt.primary.host = toStdString(src.mqtt.primary.host);
        dst.mqtt.primary.port = static_cast<int>(src.mqtt.primary.port);
        dst.mqtt.primary.user = toStdString(src.mqtt.primary.user);
        dst.mqtt.primary.password = toStdString(src.mqtt.primary.password);
        dst.mqtt.primary.protocol = toStdString(src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = static_cast<int>(src.mqtt.primary.ttl);

        dst.mqtt.secondary.host = toStdString(src.mqtt.secondary.host);
        dst.mqtt.secondary.port = static_cast<int>(src.mqtt.secondary.port);
        dst.mqtt.secondary.user = toStdString(src.mqtt.secondary.user);
        dst.mqtt.secondary.password = toStdString(src.mqtt.secondary.password);
        dst.mqtt.secondary.protocol = toStdString(src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = static_cast<int>(src.mqtt.secondary.ttl);

        dst.mqtt.announce_topic = toStdString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toStdString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toStdString(src.mqtt.notify_topic);
        dst.mqtt.profile = toStdString(src.mqtt.profile);

        dst.firmware.url = toStdString(src.firmware.url);
        dst.firmware.manifest = resolveFirmwareManifest(toStdString(src.firmware.manifest));
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = toStdString(src.firmware.update);

        dst.wifi.ssid = toStdString(src.wifi.ssid);
        dst.wifi.password = toStdString(src.wifi.password);

#ifdef IOTSMARTSYS_API_URL
        dst.api.url = IOTSMARTSYS_API_URL;
#else
        dst.api.url = toStdString(src.api.url);
#endif
        dst.api.key = toStdString(src.api.key);
        dst.api.basic_auth = toStdString(src.api.basic_auth);

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
    }

    static void fromLegacyStoredV4(const LegacyStoredSettingsV4 &src, core::settings::Settings &dst)
    {
        const auto toStdString = [](const char *value) -> std::string
        {
            return (value && *value) ? std::string(value) : std::string();
        };

        dst.in_config_mode = (src.in_config_mode != 0);
        dst.device_registered = false;

        dst.mqtt.primary.host = toStdString(src.mqtt.primary.host);
        dst.mqtt.primary.port = static_cast<int>(src.mqtt.primary.port);
        dst.mqtt.primary.user = toStdString(src.mqtt.primary.user);
        dst.mqtt.primary.password = toStdString(src.mqtt.primary.password);
        dst.mqtt.primary.protocol = toStdString(src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = static_cast<int>(src.mqtt.primary.ttl);

        dst.mqtt.secondary.host = toStdString(src.mqtt.secondary.host);
        dst.mqtt.secondary.port = static_cast<int>(src.mqtt.secondary.port);
        dst.mqtt.secondary.user = toStdString(src.mqtt.secondary.user);
        dst.mqtt.secondary.password = toStdString(src.mqtt.secondary.password);
        dst.mqtt.secondary.protocol = toStdString(src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = static_cast<int>(src.mqtt.secondary.ttl);

        dst.mqtt.announce_topic = toStdString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toStdString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toStdString(src.mqtt.notify_topic);
        dst.mqtt.profile = toStdString(src.mqtt.profile);
        if (dst.mqtt.profile.empty())
        {
            dst.mqtt.profile = "primary";
        }

        dst.firmware.url = toStdString(src.firmware.url);
        dst.firmware.manifest = resolveFirmwareManifest(toStdString(src.firmware.manifest));
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = toStdString(src.firmware.update);

        dst.wifi.ssid = toStdString(src.wifi.ssid);
        dst.wifi.password = toStdString(src.wifi.password);

#ifdef IOTSMARTSYS_API_URL
        dst.api.url = IOTSMARTSYS_API_URL;
#else
        dst.api.url = toStdString(src.api.url);
#endif
        dst.api.key = toStdString(src.api.key);
        dst.api.basic_auth = toStdString(src.api.basic_auth);

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
    }

    static void fromLegacyStoredV5(const LegacyStoredSettingsV5 &src, core::settings::Settings &dst)
    {
        const auto toStdString = [](const char *value) -> std::string
        {
            return (value && *value) ? std::string(value) : std::string();
        };

        dst.in_config_mode = (src.in_config_mode != 0);
        dst.device_registered = (src.device_registered != 0);

        dst.mqtt.primary.host = toStdString(src.mqtt.primary.host);
        dst.mqtt.primary.port = static_cast<int>(src.mqtt.primary.port);
        dst.mqtt.primary.user = toStdString(src.mqtt.primary.user);
        dst.mqtt.primary.password = toStdString(src.mqtt.primary.password);
        dst.mqtt.primary.protocol = toStdString(src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = static_cast<int>(src.mqtt.primary.ttl);

        dst.mqtt.secondary.host = toStdString(src.mqtt.secondary.host);
        dst.mqtt.secondary.port = static_cast<int>(src.mqtt.secondary.port);
        dst.mqtt.secondary.user = toStdString(src.mqtt.secondary.user);
        dst.mqtt.secondary.password = toStdString(src.mqtt.secondary.password);
        dst.mqtt.secondary.protocol = toStdString(src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = static_cast<int>(src.mqtt.secondary.ttl);

        dst.mqtt.tertiary.host = toStdString(src.mqtt.tertiary.host);
        dst.mqtt.tertiary.port = static_cast<int>(src.mqtt.tertiary.port);
        dst.mqtt.tertiary.user = toStdString(src.mqtt.tertiary.user);
        dst.mqtt.tertiary.password = toStdString(src.mqtt.tertiary.password);
        dst.mqtt.tertiary.protocol = toStdString(src.mqtt.tertiary.protocol);
        dst.mqtt.tertiary.ttl = static_cast<int>(src.mqtt.tertiary.ttl);

        dst.mqtt.announce_topic = toStdString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toStdString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toStdString(src.mqtt.notify_topic);
        dst.mqtt.profile = toStdString(src.mqtt.profile);
        if (dst.mqtt.profile.empty())
        {
            dst.mqtt.profile = "primary";
        }

        dst.firmware.url = toStdString(src.firmware.url);
        dst.firmware.manifest = resolveFirmwareManifest(toStdString(src.firmware.manifest));
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = toStdString(src.firmware.update);

        dst.wifi.ssid = toStdString(src.wifi.ssid);
        dst.wifi.password = toStdString(src.wifi.password);
        dst.wifi.primary.ssid = dst.wifi.ssid;
        dst.wifi.primary.password = dst.wifi.password;
        dst.wifi.profile = "primary";

#ifdef IOTSMARTSYS_API_URL
        dst.api.url = IOTSMARTSYS_API_URL;
#else
        dst.api.url = toStdString(src.api.url);
#endif
        dst.api.key = toStdString(src.api.key);
        dst.api.basic_auth = toStdString(src.api.basic_auth);

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
    }

    static void fromLegacyStoredV6(const LegacyStoredSettingsV6 &src, core::settings::Settings &dst)
    {
        const auto toStdString = [](const char *value) -> std::string
        {
            return (value && *value) ? std::string(value) : std::string();
        };

        dst.in_config_mode = (src.in_config_mode != 0);
        dst.device_registered = (src.device_registered != 0);

        dst.mqtt.primary.host = toStdString(src.mqtt.primary.host);
        dst.mqtt.primary.port = static_cast<int>(src.mqtt.primary.port);
        dst.mqtt.primary.user = toStdString(src.mqtt.primary.user);
        dst.mqtt.primary.password = toStdString(src.mqtt.primary.password);
        dst.mqtt.primary.protocol = toStdString(src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = static_cast<int>(src.mqtt.primary.ttl);

        dst.mqtt.secondary.host = toStdString(src.mqtt.secondary.host);
        dst.mqtt.secondary.port = static_cast<int>(src.mqtt.secondary.port);
        dst.mqtt.secondary.user = toStdString(src.mqtt.secondary.user);
        dst.mqtt.secondary.password = toStdString(src.mqtt.secondary.password);
        dst.mqtt.secondary.protocol = toStdString(src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = static_cast<int>(src.mqtt.secondary.ttl);

        dst.mqtt.tertiary.host = toStdString(src.mqtt.tertiary.host);
        dst.mqtt.tertiary.port = static_cast<int>(src.mqtt.tertiary.port);
        dst.mqtt.tertiary.user = toStdString(src.mqtt.tertiary.user);
        dst.mqtt.tertiary.password = toStdString(src.mqtt.tertiary.password);
        dst.mqtt.tertiary.protocol = toStdString(src.mqtt.tertiary.protocol);
        dst.mqtt.tertiary.ttl = static_cast<int>(src.mqtt.tertiary.ttl);

        dst.mqtt.announce_topic = toStdString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toStdString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toStdString(src.mqtt.notify_topic);
        dst.mqtt.profile = toStdString(src.mqtt.profile);
        if (dst.mqtt.profile.empty())
        {
            dst.mqtt.profile = "primary";
        }

        dst.firmware.url = toStdString(src.firmware.url);
        dst.firmware.manifest = resolveFirmwareManifest(toStdString(src.firmware.manifest));
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = toStdString(src.firmware.update);

        dst.wifi.primary.ssid = toStdString(src.wifi.primary_ssid);
        dst.wifi.primary.password = toStdString(src.wifi.primary_password);
        dst.wifi.secondary.ssid = toStdString(src.wifi.secondary_ssid);
        dst.wifi.secondary.password = toStdString(src.wifi.secondary_password);
        dst.wifi.tertiary.ssid = toStdString(src.wifi.tertiary_ssid);
        dst.wifi.tertiary.password = toStdString(src.wifi.tertiary_password);
        dst.wifi.profile = toStdString(src.wifi.profile);
        dst.wifi.ssid = toStdString(src.wifi.ssid);
        dst.wifi.password = toStdString(src.wifi.password);
        dst.wifi.syncSelectedLegacyFields();

#ifdef IOTSMARTSYS_API_URL
        dst.api.url = IOTSMARTSYS_API_URL;
#else
        dst.api.url = toStdString(src.api.url);
#endif
        dst.api.key = toStdString(src.api.key);
        dst.api.basic_auth = toStdString(src.api.basic_auth);

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
        dst.device_registered = src.device_registered ? 1 : 0;
        dst.logLevel = static_cast<int>(src.logLevel);
        dst.collect_interval_metrics = static_cast<std::int32_t>(src.collect_interval_metrics);

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
        applyMqttConfig(src.mqtt.tertiary, dst.mqtt.tertiary);

        // topics
        copyStrIfNotEmpty(dst.mqtt.announce_topic, sizeof(dst.mqtt.announce_topic), src.mqtt.announce_topic);
        copyStrIfNotEmpty(dst.mqtt.command_topic, sizeof(dst.mqtt.command_topic), src.mqtt.command_topic);
        copyStrIfNotEmpty(dst.mqtt.notify_topic, sizeof(dst.mqtt.notify_topic), src.mqtt.notify_topic);
        copyStrIfNotEmpty(dst.mqtt.profile, sizeof(dst.mqtt.profile), src.mqtt.profile);

        // firmware
        if (src.firmware.isValid())
        {
            copyStr(dst.firmware.url, sizeof(dst.firmware.url), src.firmware.url);
            copyStr(dst.firmware.manifest, sizeof(dst.firmware.manifest), src.firmware.manifest);
            dst.firmware.verify_sha256 = src.firmware.verify_sha256 ? 1 : 0;
        }

        copyStr(dst.firmware.update, sizeof(dst.firmware.update), src.firmware.update);

        // wifi (only apply when we have at least one valid profile/legacy pair)
        core::settings::WifiConfig normalizedWifi = src.wifi;
        normalizedWifi.syncSelectedLegacyFields();
        if (normalizedWifi.isValid())
        {
            copyStr(dst.wifi.primary_ssid, sizeof(dst.wifi.primary_ssid), normalizedWifi.primary.ssid);
            copyStr(dst.wifi.primary_password, sizeof(dst.wifi.primary_password), normalizedWifi.primary.password);
            copyStr(dst.wifi.secondary_ssid, sizeof(dst.wifi.secondary_ssid), normalizedWifi.secondary.ssid);
            copyStr(dst.wifi.secondary_password, sizeof(dst.wifi.secondary_password), normalizedWifi.secondary.password);
            copyStr(dst.wifi.tertiary_ssid, sizeof(dst.wifi.tertiary_ssid), normalizedWifi.tertiary.ssid);
            copyStr(dst.wifi.tertiary_password, sizeof(dst.wifi.tertiary_password), normalizedWifi.tertiary.password);
            copyStr(dst.wifi.profile, sizeof(dst.wifi.profile), normalizedWifi.profile.empty() ? std::string("primary") : normalizedWifi.profile);
            copyStr(dst.wifi.ssid, sizeof(dst.wifi.ssid), normalizedWifi.ssid);
            copyStr(dst.wifi.password, sizeof(dst.wifi.password), normalizedWifi.password);
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
        dst.device_registered = (src.device_registered != 0);
        if (src.collect_interval_metrics > 0)
        {
            dst.collect_interval_metrics = src.collect_interval_metrics;
        }

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

    // MQTT tertiary
    dst.mqtt.tertiary.host = toString(src.mqtt.tertiary.host);
    dst.mqtt.tertiary.port = static_cast<int>(src.mqtt.tertiary.port);
    dst.mqtt.tertiary.user = toString(src.mqtt.tertiary.user);
    dst.mqtt.tertiary.password = toString(src.mqtt.tertiary.password);
    dst.mqtt.tertiary.protocol = toString(src.mqtt.tertiary.protocol);
    dst.mqtt.tertiary.ttl = static_cast<int>(src.mqtt.tertiary.ttl);

        // topics
        dst.mqtt.announce_topic = toString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toString(src.mqtt.notify_topic);
        dst.mqtt.profile = toString(src.mqtt.profile);
        if (dst.mqtt.profile.empty())
        {
            dst.mqtt.profile = "primary";
        }

        // firmware
        dst.firmware.url = toString(src.firmware.url);
        dst.firmware.manifest = resolveFirmwareManifest(toString(src.firmware.manifest));
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = toString(src.firmware.update);
        dst.firmware.update = src.firmware.update;

        // wifi
        dst.wifi.primary.ssid = toString(src.wifi.primary_ssid);
        dst.wifi.primary.password = toString(src.wifi.primary_password);
        dst.wifi.secondary.ssid = toString(src.wifi.secondary_ssid);
        dst.wifi.secondary.password = toString(src.wifi.secondary_password);
        dst.wifi.tertiary.ssid = toString(src.wifi.tertiary_ssid);
        dst.wifi.tertiary.password = toString(src.wifi.tertiary_password);
        dst.wifi.profile = toString(src.wifi.profile);
        dst.wifi.ssid = toString(src.wifi.ssid);
        dst.wifi.password = toString(src.wifi.password);
        dst.wifi.syncSelectedLegacyFields();

// api
#ifdef IOTSMARTSYS_API_URL
        dst.api.url = IOTSMARTSYS_API_URL;
#else
        dst.api.url = toString(src.api.url);
#endif

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

        return (err == ESP_OK &&
                (required == sizeof(StoredSettings) ||
                 required == sizeof(LegacyStoredSettingsV6) ||
                 required == sizeof(LegacyStoredSettingsV5) ||
                 required == sizeof(LegacyStoredSettingsV4) ||
                 required == sizeof(LegacyStoredSettingsV3)));
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

        if (required != sizeof(StoredSettings) &&
            required != sizeof(LegacyStoredSettingsV6) &&
            required != sizeof(LegacyStoredSettingsV5) &&
            required != sizeof(LegacyStoredSettingsV4) &&
            required != sizeof(LegacyStoredSettingsV3))
        {
            nvs_close(h);
            return iotsmartsys::core::common::StateResult::StorageReadFail;
        }

        if (required == sizeof(StoredSettings))
        {
            StoredSettings stored{};
            err = nvs_get_blob(h, NVS_KEY, &stored, &required);
            nvs_close(h);
            if (err != ESP_OK)
                return map_esp_err(err);

            if (stored.version != STORAGE_VERSION)
                return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

            fromStored(stored, out);
            applyCompiledWifiOverride(out);
            return iotsmartsys::core::common::StateResult::Ok;
        }

        if (required == sizeof(LegacyStoredSettingsV6))
        {
            LegacyStoredSettingsV6 legacy{};
            err = nvs_get_blob(h, NVS_KEY, &legacy, &required);
            nvs_close(h);
            if (err != ESP_OK)
                return map_esp_err(err);

            if (legacy.version != 6)
                return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

            fromLegacyStoredV6(legacy, out);
            ensureClientId(out);
            applyCompiledWifiOverride(out);
            return iotsmartsys::core::common::StateResult::Ok;
        }

        if (required == sizeof(LegacyStoredSettingsV5))
        {
            LegacyStoredSettingsV5 legacy{};
            err = nvs_get_blob(h, NVS_KEY, &legacy, &required);
            nvs_close(h);
            if (err != ESP_OK)
                return map_esp_err(err);

            if (legacy.version != 5)
                return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

            fromLegacyStoredV5(legacy, out);
            ensureClientId(out);
            applyCompiledWifiOverride(out);
            return iotsmartsys::core::common::StateResult::Ok;
        }

        if (required == sizeof(LegacyStoredSettingsV4))
        {
            LegacyStoredSettingsV4 legacy{};
            err = nvs_get_blob(h, NVS_KEY, &legacy, &required);
            nvs_close(h);
            if (err != ESP_OK)
                return map_esp_err(err);

            if (legacy.version != 4)
                return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

            fromLegacyStoredV4(legacy, out);
            ensureClientId(out);
            applyCompiledWifiOverride(out);
            return iotsmartsys::core::common::StateResult::Ok;
        }

        LegacyStoredSettingsV3 legacy{};
        err = nvs_get_blob(h, NVS_KEY, &legacy, &required);
        nvs_close(h);
        if (err != ESP_OK)
            return map_esp_err(err);

        if (legacy.version != 3)
            return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

        fromLegacyStoredV3(legacy, out);
        ensureClientId(out);
        applyCompiledWifiOverride(out);
        return iotsmartsys::core::common::StateResult::Ok;
    }

    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::save(const core::settings::Settings &settings)
    {
        // _logger->info("EspIdfNvsSettingsProvider", "port mqtt: %d", settings.mqtt.primary.port);
        // _logger->info("EspIdfNvsSettingsProvider", "firmware update method: %s", settings.firmware.update.c_str());

        // _logger->info("EspIdfNvsSettingsProvider", "save() called");
        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
        {
            // _logger->error("EspIdfNvsSettingsProvider", "nvs init failed: %d", (int)err);
            return map_esp_err(err);
        }

        // Open NVS first so we can optionally preserve some fields (e.g., WiFi)
        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            // _logger->error("EspIdfNvsSettingsProvider", "nvs open failed: %d", (int)err);
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
            // _logger->error("EspIdfNvsSettingsProvider", "nvs set_blob failed: %d", (int)err);
            delete existing;
            delete stored;
            nvs_close(h);
            return map_esp_err(err);
        }
        // _logger->info("EspIdfNvsSettingsProvider", "nvs set_blob succeeded, committing...");
        err = nvs_commit(h);
        if (err != ESP_OK)
        {
            // _logger->error("EspIdfNvsSettingsProvider", "nvs commit failed: %d", (int)err);
        }
        else
        {
            // _logger->info("EspIdfNvsSettingsProvider", "nvs commit succeeded");
        }
        delete existing;
        delete stored;
        nvs_close(h);
        return map_esp_err(err);
    }

    iotsmartsys::core::common::StateResult EspIdfNvsSettingsProvider::saveWiFiOnly(const core::settings::WifiConfig &wifi)
    {

        core::settings::WifiConfig normalizedWifi = wifi;
        normalizedWifi.syncSelectedLegacyFields();
        if (!normalizedWifi.isValid())
        {
            // _logger->warn("EspIdfNvsSettingsProvider", "saveWiFiOnly: ignoring empty WiFi config (ssid/password not provided)");
            return StateResult::InvalidArg;
        }

        esp_err_t err = ensureNvsInit();
        if (err != ESP_OK)
        {
            // _logger->error("EspIdfNvsSettingsProvider", "nvs init failed: %d", (int)err);
            return map_esp_err(err);
        }

        nvs_handle_t h;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
        if (err != ESP_OK)
        {
            // _logger->error("EspIdfNvsSettingsProvider", "nvs open failed: %d", (int)err);
            return map_esp_err(err);
        }

        // Load existing settings (if present). If not present, create a fresh StoredSettings
        // so we can persist WiFi-only fields on first boot. Use heap allocation to avoid
        // exhausting the FreeRTOS task stack.
        StoredSettings *stored = new StoredSettings();
        size_t required = 0;
        err = nvs_get_blob(h, NVS_KEY, nullptr, &required);
        if (err == ESP_OK && required == sizeof(StoredSettings))
        {
            err = nvs_get_blob(h, NVS_KEY, stored, &required);
            if (err != ESP_OK || stored->version != STORAGE_VERSION)
            {
                std::memset(stored, 0, sizeof(*stored));
                stored->version = STORAGE_VERSION;
            }
        }
        else if (err == ESP_OK && required == sizeof(LegacyStoredSettingsV6))
        {
            LegacyStoredSettingsV6 legacy{};
            err = nvs_get_blob(h, NVS_KEY, &legacy, &required);
            if (err == ESP_OK && legacy.version == 6)
            {
                core::settings::Settings migrated{};
                fromLegacyStoredV6(legacy, migrated);
                toStored(migrated, *stored, nullptr);
            }
            else
            {
                std::memset(stored, 0, sizeof(*stored));
                stored->version = STORAGE_VERSION;
            }
        }
        else
        {
            // No existing blob or invalid size: create a new default-stored object.

            std::memset(stored, 0, sizeof(*stored));
            stored->version = STORAGE_VERSION;
            // continue — we'll set wifi fields below and persist the new blob
        }

        // Update only WiFi settings
        copyStr(stored->wifi.primary_ssid, sizeof(stored->wifi.primary_ssid), normalizedWifi.primary.ssid);
        copyStr(stored->wifi.primary_password, sizeof(stored->wifi.primary_password), normalizedWifi.primary.password);
        copyStr(stored->wifi.secondary_ssid, sizeof(stored->wifi.secondary_ssid), normalizedWifi.secondary.ssid);
        copyStr(stored->wifi.secondary_password, sizeof(stored->wifi.secondary_password), normalizedWifi.secondary.password);
        copyStr(stored->wifi.tertiary_ssid, sizeof(stored->wifi.tertiary_ssid), normalizedWifi.tertiary.ssid);
        copyStr(stored->wifi.tertiary_password, sizeof(stored->wifi.tertiary_password), normalizedWifi.tertiary.password);
        copyStr(stored->wifi.profile, sizeof(stored->wifi.profile), normalizedWifi.profile.empty() ? std::string("primary") : normalizedWifi.profile);
        copyStr(stored->wifi.ssid, sizeof(stored->wifi.ssid), normalizedWifi.ssid);
        // Never log passwords; optionally log only length for diagnostics (removed debug logs)

        copyStr(stored->wifi.password, sizeof(stored->wifi.password), normalizedWifi.password);

        // Debug: show what we'll write for WiFi (without password content)

        // Save back
        err = nvs_set_blob(h, NVS_KEY, stored, sizeof(*stored));
        if (err != ESP_OK)
        {
            // _logger->error("EspIdfNvsSettingsProvider", "nvs set_blob failed: %d", (int)err);
            delete stored;
            nvs_close(h);
            return map_esp_err(err);
        }

        err = nvs_commit(h);

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
                }
                else
                {
                    // _logger->warn("EspIdfNvsSettingsProvider", "Readback failed: err=%d vreq=%u", (int)rerr, (unsigned)vreq);
                }
                delete verify;
                nvs_close(hr);
            }
            else
            {
                // _logger->warn("EspIdfNvsSettingsProvider", "Readback nvs_open failed: %d", (int)rerr);
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
            // _logger->error("EspIdfNvsSettingsProvider", "nvs open failed: %d", (int)err);
            return map_esp_err(err);
        }

        err = nvs_erase_key(h, NVS_KEY);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            // _logger->warn("EspIdfNvsSettingsProvider", "nvs key not found: %d", (int)err);
            err = ESP_OK;
        }

        if (err == ESP_OK)
        {
            // _logger->info("EspIdfNvsSettingsProvider", "nvs key erased successfully");
            err = nvs_commit(h);
        }
        nvs_close(h);
        return map_esp_err(err);
    }
} // namespace iotsmartsys::platform::espressif

#endif // ESP32
