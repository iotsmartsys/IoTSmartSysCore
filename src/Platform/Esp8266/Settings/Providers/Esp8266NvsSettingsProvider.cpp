#ifdef ESP8266
#include "Esp8266NvsSettingsProvider.h"

#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"

#include <EEPROM.h>
#include <cstring>
#include <algorithm>

namespace iotsmartsys::platform::esp8266
{
    using namespace iotsmartsys::core;
    using core::common::StateResult;

    // EEPROM size will be the size of the stored settings blob.

    Esp8266NvsSettingsProvider::Esp8266NvsSettingsProvider()
        : _logger(iotsmartsys::core::ServiceProvider::instance().logger()),
          _deviceIdentityProvider()
    {
    }

    void Esp8266NvsSettingsProvider::copyStr(char *dst, std::size_t dstSize, const std::string &src)
    {
        if (!dst || dstSize == 0)
            return;
        const std::size_t n = std::min(dstSize - 1, src.size());
        std::memcpy(dst, src.data(), n);
        dst[n] = '\0';
    }

    void Esp8266NvsSettingsProvider::copyStrIfNotEmpty(char *dst, std::size_t dstSize, const std::string &src)
    {
        if (!src.empty())
            copyStr(dst, dstSize, src);
    }

    std::string Esp8266NvsSettingsProvider::toString(const char *src)
    {
        return (src && *src) ? std::string(src) : std::string();
    }

    void Esp8266NvsSettingsProvider::toStored(const core::settings::Settings &src, StoredSettings &dst, const StoredSettings *existing)
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

        copyStrIfNotEmpty(dst.mqtt.announce_topic, sizeof(dst.mqtt.announce_topic), src.mqtt.announce_topic);
        copyStrIfNotEmpty(dst.mqtt.command_topic, sizeof(dst.mqtt.command_topic), src.mqtt.command_topic);
        copyStrIfNotEmpty(dst.mqtt.notify_topic, sizeof(dst.mqtt.notify_topic), src.mqtt.notify_topic);

        if (src.firmware.isValid())
        {
            copyStr(dst.firmware.url, sizeof(dst.firmware.url), src.firmware.url);
            copyStr(dst.firmware.manifest, sizeof(dst.firmware.manifest), src.firmware.manifest);
            dst.firmware.verify_sha256 = src.firmware.verify_sha256 ? 1 : 0;
        }

        copyStr(dst.firmware.update, sizeof(dst.firmware.update), src.firmware.update);

        if (!src.wifi.ssid.empty() && !src.wifi.password.empty())
        {
            copyStr(dst.wifi.ssid, sizeof(dst.wifi.ssid), src.wifi.ssid);
            copyStr(dst.wifi.password, sizeof(dst.wifi.password), src.wifi.password);
        }

        if (src.api.isValid())
        {
            copyStr(dst.api.url, sizeof(dst.api.url), src.api.url);
            copyStr(dst.api.key, sizeof(dst.api.key), src.api.key);
            copyStr(dst.api.basic_auth, sizeof(dst.api.basic_auth), src.api.basic_auth);
        }
    }

    void Esp8266NvsSettingsProvider::fromStored(const StoredSettings &src, core::settings::Settings &dst)
    {
        _clientIdCache = _deviceIdentityProvider.getDeviceID();
        dst.clientId = _clientIdCache.c_str();
        dst.in_config_mode = (src.in_config_mode != 0);

        dst.mqtt.primary.host = toString(src.mqtt.primary.host);
        dst.mqtt.primary.port = static_cast<int>(src.mqtt.primary.port);
        dst.mqtt.primary.user = toString(src.mqtt.primary.user);
        dst.mqtt.primary.password = toString(src.mqtt.primary.password);
        dst.mqtt.primary.protocol = toString(src.mqtt.primary.protocol);
        dst.mqtt.primary.ttl = static_cast<int>(src.mqtt.primary.ttl);

        dst.mqtt.secondary.host = toString(src.mqtt.secondary.host);
        dst.mqtt.secondary.port = static_cast<int>(src.mqtt.secondary.port);
        dst.mqtt.secondary.user = toString(src.mqtt.secondary.user);
        dst.mqtt.secondary.password = toString(src.mqtt.secondary.password);
        dst.mqtt.secondary.protocol = toString(src.mqtt.secondary.protocol);
        dst.mqtt.secondary.ttl = static_cast<int>(src.mqtt.secondary.ttl);

        dst.mqtt.announce_topic = toString(src.mqtt.announce_topic);
        dst.mqtt.command_topic = toString(src.mqtt.command_topic);
        dst.mqtt.notify_topic = toString(src.mqtt.notify_topic);

        dst.firmware.url = toString(src.firmware.url);
        dst.firmware.manifest = toString(src.firmware.manifest);
        dst.firmware.verify_sha256 = (src.firmware.verify_sha256 != 0);
        dst.firmware.update = toString(src.firmware.update);

        dst.wifi.ssid = toString(src.wifi.ssid);
        dst.wifi.password = toString(src.wifi.password);

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
    }

    bool Esp8266NvsSettingsProvider::exists()
    {
        // EEPROM.begin() returns void on ESP8266 core. Call it and inspect stored blob.
        EEPROM.begin(sizeof(StoredSettings));

        StoredSettings stored{};
        EEPROM.get(0, stored);
        EEPROM.end();
        return (stored.version == STORAGE_VERSION);
    }

    iotsmartsys::core::common::StateResult Esp8266NvsSettingsProvider::load(core::settings::Settings &out)
    {
        // EEPROM.begin() returns void on ESP8266 core.
        EEPROM.begin(sizeof(StoredSettings));

        StoredSettings stored{};
        EEPROM.get(0, stored);
        EEPROM.end();

        if (stored.version != STORAGE_VERSION)
            return iotsmartsys::core::common::StateResult::StorageVersionMismatch;

        fromStored(stored, out);
        return iotsmartsys::core::common::StateResult::Ok;
    }

    iotsmartsys::core::common::StateResult Esp8266NvsSettingsProvider::save(const core::settings::Settings &settings)
    {
    _logger->info("Esp8266NvsSettingsProvider", "save() called");
        EEPROM.begin(sizeof(StoredSettings));
        // Read existing stored (if any) to preserve fields
        StoredSettings existing{};
        EEPROM.get(0, existing);
        bool hasExisting = (existing.version == STORAGE_VERSION);

        StoredSettings stored{};
        toStored(settings, stored, hasExisting ? &existing : nullptr);
        EEPROM.put(0, stored);
        bool ok = EEPROM.commit();
        EEPROM.end();
        

        return ok ? iotsmartsys::core::common::StateResult::Ok : iotsmartsys::core::common::StateResult::StorageWriteFail;
    }

    iotsmartsys::core::common::StateResult Esp8266NvsSettingsProvider::saveWiFiOnly(const core::settings::WifiConfig &wifi)
    {
        _logger->debug("Esp8266NvsSettingsProvider", "saveWiFiOnly() called");
        if (wifi.ssid.empty() || wifi.password.empty())
        {
            _logger->warn("Esp8266NvsSettingsProvider", "saveWiFiOnly: ignoring empty WiFi config (ssid/password not provided)");
            return StateResult::InvalidArg;
        }

        EEPROM.begin(sizeof(StoredSettings));

        StoredSettings stored{};
        EEPROM.get(0, stored);
        if (stored.version != STORAGE_VERSION)
        {
            std::memset(&stored, 0, sizeof(stored));
            stored.version = STORAGE_VERSION;
        }

        copyStr(stored.wifi.ssid, sizeof(stored.wifi.ssid), wifi.ssid);
        copyStr(stored.wifi.password, sizeof(stored.wifi.password), wifi.password);

        EEPROM.put(0, stored);
        bool ok = EEPROM.commit();
        EEPROM.end();

        return ok ? iotsmartsys::core::common::StateResult::Ok : iotsmartsys::core::common::StateResult::StorageWriteFail;
    }

    iotsmartsys::core::common::StateResult Esp8266NvsSettingsProvider::erase()
    {
        EEPROM.begin(sizeof(StoredSettings));

        StoredSettings zero{};
        std::memset(&zero, 0, sizeof(zero));
        EEPROM.put(0, zero);
        bool ok = EEPROM.commit();
        EEPROM.end();
        return ok ? iotsmartsys::core::common::StateResult::Ok : iotsmartsys::core::common::StateResult::StorageWriteFail;
    }
}
#endif // ESP8266
