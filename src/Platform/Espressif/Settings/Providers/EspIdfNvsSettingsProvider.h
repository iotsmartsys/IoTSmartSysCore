// Platform/Espressif/Settings/EspIdfNvsSettingsProvider.h
#pragma once

#include "Contracts/Providers/ISettingsProvider.h"
#include "Contracts/Logging/ILogger.h"

extern "C"
{
#include "nvs.h"
#include "nvs_flash.h"
}

namespace iotsmartsys::platform::espressif
{
    class EspIdfNvsSettingsProvider final : public core::providers::ISettingsProvider
    {
    public:
        EspIdfNvsSettingsProvider();
        ~EspIdfNvsSettingsProvider() override = default;

        iotsmartsys::core::common::StateResult load(core::settings::Settings &out) override;
        iotsmartsys::core::common::StateResult save(const core::settings::Settings &settings) override;
        iotsmartsys::core::common::StateResult saveWiFiOnly(const core::settings::WifiConfig &wifi) override;
        iotsmartsys::core::common::StateResult erase() override;
        bool exists() override;

    private:
        iotsmartsys::core::ILogger *_logger;
        static constexpr const char *NVS_NAMESPACE = "iotsys";
        static constexpr const char *NVS_KEY = "settings";

        static constexpr std::uint32_t STORAGE_VERSION = 2;

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
            std::uint8_t update; // 0 NONE, 1 OTA, 2 AUTO
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
        static void toStored(const core::settings::Settings &src, StoredSettings &dst, const StoredSettings *existing = nullptr);
        static void fromStored(const StoredSettings &src, core::settings::Settings &dst);

        // helpers
        static void copyStr(char *dst, std::size_t dstSize, const std::string &src);
        static void copyStrIfNotEmpty(char *dst, std::size_t dstSize, const std::string &src);
        static std::string toString(const char *src);

        static esp_err_t ensureNvsInit();
    };
} // namespace iotsmartsys::platform::espressif
