// Platform/Espressif/Settings/EspIdfNvsSettingsProvider.h
#pragma once

#include "Contracts/Providers/ISettingsProvider.h"

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
        EspIdfNvsSettingsProvider() = default;
        ~EspIdfNvsSettingsProvider() override = default;

        esp_err_t load(core::settings::Settings &out) override;
        esp_err_t save(const core::settings::Settings &settings) override;
        esp_err_t erase() override;
        bool exists() override;

    private:
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
            char key[128];
            char basic_auth[160];
        };

        struct StoredSettings
        {
            std::uint32_t version;
            std::uint8_t in_config_mode;

            StoredMqttSettings mqtt;
            StoredFirmwareConfig firmware;
            StoredWifiConfig wifi;
            StoredApiConfig api;
        };

        // conversões
        static void toStored(const core::settings::Settings &src, StoredSettings &dst);
        static void fromStored(const StoredSettings &src, core::settings::Settings &dst);

        // helpers
        static void copyStr(char *dst, std::size_t dstSize, const std::string &src);
        static std::string toString(const char *src);

        static esp_err_t ensureNvsInit();
    };
} // namespace iotsmartsys::platform::espressif