#include "SettingsFetcher.h"
#include "Utils/Logger.h"
#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

String SettingsFetcher::buildUrl(const String &deviceId) const
{
    String base = String(SETTINGS_API_BASE_URL);

    if (base.endsWith("/"))
        base.remove(base.length() - 1);

    String url = base;
    url += "/devices/" + deviceId + "/settings";

    return url;
}

bool SettingsFetcher::fetchSettings(const String &deviceId,
                                    Settings &settings,
                                    unsigned long timeoutMs) const
{
    if (deviceId.length() == 0)
    {
        LOG_ERROR("Device ID is empty");
        return false;
    }

    String url = buildUrl(deviceId);
    LOG_INFO("Fetching settings from: " + url);
#ifdef ESP32
    WiFiClientSecure *client = new WiFiClientSecure();
    client->setInsecure();
#else
    WiFiClient *client = new WiFiClient();
#endif
    HTTPClient http;
    http.setTimeout(timeoutMs);

    bool success = false;
    if (http.begin(*client, url))
    {
        http.addHeader("x-api-key", settings.api.key);
        http.addHeader("client_id", deviceId);
        http.addHeader("Authorization", String("Basic ") + String(settings.api.basic_auth));
    }

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        LOG_INFO("Settings fetched successfully");
        String payload = http.getString();

        LOG_PRINTLN(F("Settings recebido:"));
        LOG_PRINTLN(payload);
        success = parseSettingsJson(payload, settings);
    }
    else
    {
        // HTTP retornou erro
        LOG_ERROR("Failed to fetch settings, HTTP code: " + String(httpCode));
        LOG_PRINTF(
            "[HTTP] GET failed, code: %d, error: %s\n",
            httpCode,
            http.errorToString(httpCode).c_str());
        success = false;
    }

    http.end();
    delete client;
    return success;
}
bool SettingsFetcher::parseSettingsJson(const String &payload, Settings &settings) const
{
    auto parseMqttConfig = [](JsonObject obj, MqttConfig &config)
    {
        if (obj.isNull())
        {
            return;
        }

        if (obj.containsKey("host"))
        {
            config.host = String((const char *)obj["host"]);
        }
        if (obj.containsKey("port"))
        {
            config.port = obj["port"].as<int>();
        }
        if (obj.containsKey("user"))
        {
            config.user = String((const char *)obj["user"]);
        }
        if (obj.containsKey("password"))
        {
            config.password = String((const char *)obj["password"]);
        }
        if (obj.containsKey("protocol"))
        {
            config.protocol = String((const char *)obj["protocol"]);
        }
        if (obj.containsKey("ttl"))
        {
            config.ttl = obj["ttl"].as<int>();
        }
    };

    bool success = false;
    const size_t capacity = 2048;
    DynamicJsonDocument doc(capacity);
    DeserializationError err = deserializeJson(doc, payload);
    if (!err)
    {
        if (doc.containsKey("mqtt"))
        {
            JsonObject mqttObj = doc["mqtt"].as<JsonObject>();
            parseMqttConfig(mqttObj["primary"].as<JsonObject>(), settings.mqtt.primary);
            parseMqttConfig(mqttObj["secondary"].as<JsonObject>(), settings.mqtt.secondary);
        }

        // firmware.url, firmware.manifest, firmware.verifysha256
        if (doc.containsKey("firmware"))
        {
            JsonObject fwObj = doc["firmware"].as<JsonObject>();
            if (!fwObj.isNull())
            {
                if (fwObj.containsKey("url"))
                {
                    settings.firmware.url = String((const char *)fwObj["url"]);
                }

                if (fwObj.containsKey("manifest"))
                {
                    settings.firmware.manifest = String((const char *)fwObj["manifest"]);
                }

                if (fwObj.containsKey("verifysha256"))
                {
                    settings.firmware.verify_sha256 = fwObj["verifysha256"].as<bool>();
                }

                if (fwObj.containsKey("update"))
                {
                    settings.firmware.setUpdate((const char *)fwObj["update"]);
                }
            }
        }

        // if (doc.containsKey("ota_enabled"))
        //     out.ota_enabled = doc["ota_enabled"].as<bool>();

        success = true;
    }
    else
    {
        // Falha no parse
        LOG_ERROR("Failed to parse settings JSON: " + String(err.c_str()));
        success = false;
    }

    return success;
}
