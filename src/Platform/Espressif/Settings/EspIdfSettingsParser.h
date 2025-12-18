#pragma once

#include "Contracts/Settings/ISettingsParser.h"

namespace iotsmartsys::platform::espressif
{
    class EspIdfSettingsParser final : public iotsmartsys::core::settings::ISettingsParser
    {
    public:
        EspIdfSettingsParser() = default;
        ~EspIdfSettingsParser() override = default;

        iotsmartsys::core::common::Error parse(const char *json, iotsmartsys::core::settings::Settings &out) override;

    private:
        static bool jsonGetString(void *obj, const char *key, std::string &out);
        static bool jsonGetInt(void *obj, const char *key, int &out);
        static bool jsonGetBool(void *obj, const char *key, bool &out);

    static iotsmartsys::core::common::Error parseMqtt(void *mqttObj, iotsmartsys::core::settings::MqttSettings &out);
    static iotsmartsys::core::common::Error parseMqttConfig(void *cfgObj, iotsmartsys::core::settings::MqttConfig &out, bool allowTtl);
    static iotsmartsys::core::common::Error parseFirmware(void *fwObj, iotsmartsys::core::settings::FirmwareConfig &out);
    static iotsmartsys::core::common::Error parseWifi(void *wifiObj, iotsmartsys::core::settings::WifiConfig &out);
    static iotsmartsys::core::common::Error parseApi(void *apiObj, iotsmartsys::core::settings::ApiConfig &out);
    };
} // namespace iotsmartsys::platform::espressif