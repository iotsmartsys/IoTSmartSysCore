#pragma once

#include "Contracts/Settings/ISettingsParser.h"

namespace iotsmartsys::platform::espressif
{
    class EspIdfSettingsParser final : public iotsmartsys::core::settings::ISettingsParser
    {
    public:
        EspIdfSettingsParser() = default;
        ~EspIdfSettingsParser() override = default;

        esp_err_t parse(const char *json, iotsmartsys::core::settings::Settings &out) override;

    private:
        static bool jsonGetString(void *obj, const char *key, std::string &out);
        static bool jsonGetInt(void *obj, const char *key, int &out);
        static bool jsonGetBool(void *obj, const char *key, bool &out);

        static esp_err_t parseMqtt(void *mqttObj, iotsmartsys::core::settings::MqttSettings &out);
        static esp_err_t parseMqttConfig(void *cfgObj, iotsmartsys::core::settings::MqttConfig &out, bool allowTtl);
        static esp_err_t parseFirmware(void *fwObj, iotsmartsys::core::settings::FirmwareConfig &out);
        static esp_err_t parseWifi(void *wifiObj, iotsmartsys::core::settings::WifiConfig &out);
        static esp_err_t parseApi(void *apiObj, iotsmartsys::core::settings::ApiConfig &out);
    };
} // namespace iotsmartsys::platform::espressif