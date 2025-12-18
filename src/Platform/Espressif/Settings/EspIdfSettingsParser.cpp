
#include "EspIdfSettingsParser.h"

#include <cstring>

extern "C"
{
#include "cJSON.h"
}

namespace iotsmartsys::platform::espressif
{
    using namespace iotsmartsys::core::settings;

    // Helpers cJSON
    bool EspIdfSettingsParser::jsonGetString(void *obj, const char *key, std::string &out)
    {
        auto *o = static_cast<cJSON *>(obj);
        cJSON *v = cJSON_GetObjectItemCaseSensitive(o, key);
        if (!cJSON_IsString(v) || !v->valuestring)
            return false;
        out.assign(v->valuestring);
        return true;
    }

    bool EspIdfSettingsParser::jsonGetInt(void *obj, const char *key, int &out)
    {
        auto *o = static_cast<cJSON *>(obj);
        cJSON *v = cJSON_GetObjectItemCaseSensitive(o, key);
        if (!cJSON_IsNumber(v))
            return false;
        out = v->valueint;
        return true;
    }

    bool EspIdfSettingsParser::jsonGetBool(void *obj, const char *key, bool &out)
    {
        auto *o = static_cast<cJSON *>(obj);
        cJSON *v = cJSON_GetObjectItemCaseSensitive(o, key);
        if (!cJSON_IsBool(v))
            return false;
        out = cJSON_IsTrue(v);
        return true;
    }

    esp_err_t EspIdfSettingsParser::parseMqttConfig(void *cfgObj, MqttConfig &out, bool allowTtl)
    {
        auto *cfg = static_cast<cJSON *>(cfgObj);
        if (!cJSON_IsObject(cfg))
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        // defaults
        out = MqttConfig{};
        out.port = 1883;
        out.protocol = "mqtt";
        out.ttl = 0;

        // host obrigatório
        if (!jsonGetString(cfg, "host", out.host) || out.host.empty())
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        // port obrigatório no seu JSON (mas vamos aceitar default se vier ausente)
        int port = 1883;
        if (jsonGetInt(cfg, "port", port))
        {
            if (port <= 0 || port > 65535)
                return ESP_ERR_SETTINGS_PARSE_OUT_OF_RANGE;
            out.port = port;
        }

        // user/password opcionais (mas normalmente presentes)
        (void)jsonGetString(cfg, "user", out.user);
        (void)jsonGetString(cfg, "password", out.password);

        // protocol opcional (default mqtt)
        (void)jsonGetString(cfg, "protocol", out.protocol);
        if (out.protocol.empty())
            out.protocol = "mqtt";

        // ttl apenas para secondary no seu JSON (mas pode existir em ambos)
        if (allowTtl)
        {
            int ttl = 0;
            if (jsonGetInt(cfg, "ttl", ttl))
            {
                // Guard rails: TTL em minutos, 0..1440 (1 dia) por segurança
                if (ttl < 0 || ttl > 1440)
                    return ESP_ERR_SETTINGS_PARSE_OUT_OF_RANGE;
                out.ttl = ttl;
            }
        }

        // validação mínima final
        if (!out.isValid())
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        return ESP_OK;
    }

    esp_err_t EspIdfSettingsParser::parseMqtt(void *mqttObj, MqttSettings &out)
    {
        auto *mqtt = static_cast<cJSON *>(mqttObj);
        if (!cJSON_IsObject(mqtt))
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        cJSON *primary = cJSON_GetObjectItemCaseSensitive(mqtt, "primary");
        cJSON *secondary = cJSON_GetObjectItemCaseSensitive(mqtt, "secondary");
        cJSON *topic = cJSON_GetObjectItemCaseSensitive(mqtt, "topic");

        // primary obrigatório
        esp_err_t err = parseMqttConfig(primary, out.primary, /*allowTtl*/ false);
        if (err != ESP_OK)
            return err;

        // secondary opcional
        if (cJSON_IsObject(secondary))
        {
            err = parseMqttConfig(secondary, out.secondary, /*allowTtl*/ true);
            if (err != ESP_OK)
                return err;
        }
        else
        {
            out.secondary = MqttConfig{}; // limpa
        }

        // topics opcionais (defaults já existem no model)
        if (cJSON_IsObject(topic))
        {
            (void)jsonGetString(topic, "announce", out.announce_topic);
            (void)jsonGetString(topic, "command", out.command_topic);
            (void)jsonGetString(topic, "notify", out.notify_topic);
        }

        return ESP_OK;
    }

    esp_err_t EspIdfSettingsParser::parseFirmware(void *fwObj, FirmwareConfig &out)
    {
        auto *fw = static_cast<cJSON *>(fwObj);
        if (!cJSON_IsObject(fw))
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        std::string url, manifest, update;
        bool verify = false;

        if (!jsonGetString(fw, "url", url) || url.empty())
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        if (!jsonGetString(fw, "manifest", manifest) || manifest.empty())
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        (void)jsonGetBool(fw, "verifysha256", verify);

        // update é string no JSON: "auto" / "ota" / "none"
        if (jsonGetString(fw, "update", update))
        {
            out.setUpdate(update.c_str());
        }
        else
        {
            out.update = FirmwareUpdateMethod::OTA; // default do seu model
        }

        out.url = url;
        out.manifest = manifest;
        out.verify_sha256 = verify;

        return ESP_OK;
    }

    esp_err_t EspIdfSettingsParser::parseWifi(void *wifiObj, WifiConfig &out)
    {
        auto *wifi = static_cast<cJSON *>(wifiObj);
        if (!cJSON_IsObject(wifi))
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        // No JSON que você mostrou, wifi não veio (você disse que esse JSON é exatamente o que vai gravar).
        // Então: se não existir, não falha. Aqui a função assume que foi chamada só se existir.
        (void)jsonGetString(wifi, "ssid", out.ssid);
        (void)jsonGetString(wifi, "password", out.password);
        return ESP_OK;
    }

    esp_err_t EspIdfSettingsParser::parseApi(void *apiObj, ApiConfig &out)
    {
        auto *api = static_cast<cJSON *>(apiObj);
        if (!cJSON_IsObject(api))
            return ESP_ERR_SETTINGS_PARSE_MISSING_FIELD;

        (void)jsonGetString(api, "key", out.key);
        (void)jsonGetString(api, "basic_auth", out.basic_auth);
        return ESP_OK;
    }

    esp_err_t EspIdfSettingsParser::parse(const char *json, Settings &out)
    {
        if (!json || *json == '\0')
            return ESP_ERR_SETTINGS_PARSE_INVALID_JSON;

        cJSON *root = cJSON_Parse(json);
        if (!root)
            return ESP_ERR_SETTINGS_PARSE_INVALID_JSON;

        // Garantia de cleanup em qualquer return:
        esp_err_t result = ESP_OK;

        // mqtt obrigatório
        cJSON *mqtt = cJSON_GetObjectItemCaseSensitive(root, "mqtt");
        result = parseMqtt(mqtt, out.mqtt);
        if (result != ESP_OK)
        {
            cJSON_Delete(root);
            return result;
        }

        // firmware obrigatório (no seu JSON atual)
        cJSON *fw = cJSON_GetObjectItemCaseSensitive(root, "firmware");
        result = parseFirmware(fw, out.firmware);
        if (result != ESP_OK)
        {
            cJSON_Delete(root);
            return result;
        }

        // wifi opcional (não existe no JSON que você mostrou)
        cJSON *wifi = cJSON_GetObjectItemCaseSensitive(root, "wifi");
        if (cJSON_IsObject(wifi))
        {
            (void)parseWifi(wifi, out.wifi);
        }

        // api opcional (não existe no JSON que você mostrou)
        cJSON *api = cJSON_GetObjectItemCaseSensitive(root, "api");
        if (cJSON_IsObject(api))
        {
            (void)parseApi(api, out.api);
        }

        // in_config_mode opcional
        cJSON *icm = cJSON_GetObjectItemCaseSensitive(root, "in_config_mode");
        if (cJSON_IsBool(icm))
            out.in_config_mode = cJSON_IsTrue(icm);

        // prefix_auto_format_properies_json (typo no nome no JSON)
        cJSON *pref = cJSON_GetObjectItemCaseSensitive(root, "prefix_auto_format_properies_json");
        if (cJSON_IsString(pref) && pref->valuestring)
        {
            // você não tem isso no model Settings atual (só existe no JSON),
            // então aqui eu não salvo em lugar nenhum.
            // Se você quiser, adicionamos no model.
        }

        cJSON_Delete(root);
        return ESP_OK;
    }
} // namespace iotsmartsys::platform::espressif