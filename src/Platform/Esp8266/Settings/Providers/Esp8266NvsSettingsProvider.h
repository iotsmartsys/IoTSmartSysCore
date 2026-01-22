#pragma once
#ifdef ESP8266
#include "Contracts/Providers/ISettingsProvider.h"
#include "Contracts/Logging/ILogger.h"
#include "Platform/Espressif/Providers/DeviceIdentityProvider.h"

#include <string>

// EEPROM will be used as storage backend on ESP8266

namespace iotsmartsys::platform::esp8266
{
    class Esp8266NvsSettingsProvider final : public core::providers::ISettingsProvider
    {
    public:
        Esp8266NvsSettingsProvider();
        ~Esp8266NvsSettingsProvider() override = default;

        iotsmartsys::core::common::StateResult load(iotsmartsys::core::settings::Settings &out) override;
        iotsmartsys::core::common::StateResult save(const iotsmartsys::core::settings::Settings &settings) override;
        iotsmartsys::core::common::StateResult saveWiFiOnly(const iotsmartsys::core::settings::WifiConfig &wifi) override;
        iotsmartsys::core::common::StateResult erase() override;
        bool exists() override;

    private:
        iotsmartsys::core::ILogger *_logger;
        platform::espressif::providers::DeviceIdentityProvider _deviceIdentityProvider;
        std::string _clientIdCache;
        static constexpr const char *NVS_NAMESPACE = "iotsys";
        static constexpr const char *NVS_KEY = "settings";

        static constexpr std::uint32_t STORAGE_VERSION = 3;

        // --- POD storage model (persistível) ---

        struct StoredMqttConfig
        {
            char host[128];
            std::int32_t port; // novo
            char user[64];     // novo
            char password[64];
            char protocol[8];
            std::int32_t ttl; // minutos (0 = desativado)
        };

        struct StoredMqttSettings
        {
            StoredMqttConfig primary;
            StoredMqttConfig secondary;
            char announce_topic[128];
            char command_topic[128];
            char notify_topic[128];
        };

        struct StoredFirmwareConfig
        {
            char url[128];
            char manifest[160];
            std::uint8_t verify_sha256;
            char update[8]; // "none", "ota", "auto"
        };

        struct StoredWifiConfig
        {
            char ssid[64];
            char password[64];
        };

        struct StoredApiConfig
        {
            char url[128];
            char key[128];
            char basic_auth[160];
        };

        struct StoredSettings
        {
            std::uint32_t version;
            std::uint8_t in_config_mode;
            std::int32_t logLevel;

            StoredMqttSettings mqtt;
            StoredFirmwareConfig firmware;
            StoredWifiConfig wifi;
            StoredApiConfig api;
        };

        // conversões
        void toStored(const core::settings::Settings &src, StoredSettings &dst, const StoredSettings *existing = nullptr);
        void fromStored(const StoredSettings &src, core::settings::Settings &dst);

        // helpers
        void copyStr(char *dst, std::size_t dstSize, const std::string &src);
        void copyStrIfNotEmpty(char *dst, std::size_t dstSize, const std::string &src);
        std::string toString(const char *src);
    };
} // namespace iotsmartsys::platform::esp8266

#endif // ESP8266
