#include <ArduinoJson.h>
#if defined(ESP32)
#include <Preferences.h>
#elif defined(ESP8266)
#include <LittleFS.h>
#endif
#include "ConfigManager.h"
#include "Settings/SettingsFetcher.h"
#include "Wifi/WifiHelper.h"
#include "Utils/Logger.h"

#if defined(ESP32)
Preferences preferences;
#elif defined(ESP8266)
static const char *PREF_FILE = "/config.json";
#endif
SettingsFetcher *settingsFetcher = new SettingsFetcher();

bool ConfigManager::updateConfig(const Settings &settings)
{
    g_config = settings;
    saveConfigToPreferences();
    return true;
}

bool ConfigManager::saveConfigToPreferences()
{
#if defined(ESP32)
    if (!preferences.begin(PREF_NAMESPACE, false))
    {
        LOG_PRINT("Prefs begin failed");
        return false;
    }

    // MQTT Primary
    appendMqttPreferences(PREF_PREFIX_MQTT_PRIMARY, g_config.mqtt.primary);
    // MQTT Secondary
    appendMqttPreferences(PREF_PREFIX_MQTT_SECONDARY, g_config.mqtt.secondary);
    // Firmware
    appendFirmwarePreferences(g_config.firmware);
    // WiFi
    appendWifiPreferences(g_config.wifi);
    // API
    appendApiPreferences(g_config.api);

    preferences.putBool(PREF_KEY_IN_CONFIG_MODE, g_config.in_config_mode);

    preferences.end();
    return true;
#elif defined(ESP8266)
    if (!LittleFS.begin())
    {
        LOG_PRINT("LittleFS mount failed");
        return false;
    }

    DynamicJsonDocument doc(2048);

    JsonObject mqttPrimary = doc.createNestedObject("mqtt_primary");
    mqttPrimary["host"] = g_config.mqtt.primary.host;
    mqttPrimary["port"] = g_config.mqtt.primary.port;
    mqttPrimary["user"] = g_config.mqtt.primary.user;
    mqttPrimary["password"] = g_config.mqtt.primary.password;
    mqttPrimary["protocol"] = g_config.mqtt.primary.protocol;
    mqttPrimary["ttl"] = g_config.mqtt.primary.ttl;

    JsonObject mqttSecondary = doc.createNestedObject("mqtt_secondary");
    mqttSecondary["host"] = g_config.mqtt.secondary.host;
    mqttSecondary["port"] = g_config.mqtt.secondary.port;
    mqttSecondary["user"] = g_config.mqtt.secondary.user;
    mqttSecondary["password"] = g_config.mqtt.secondary.password;
    mqttSecondary["protocol"] = g_config.mqtt.secondary.protocol;
    mqttSecondary["ttl"] = g_config.mqtt.secondary.ttl;

    JsonObject firmware = doc.createNestedObject("firmware");
    firmware["url"] = g_config.firmware.url;
    firmware["manifest"] = g_config.firmware.manifest;
    firmware["verify_sha256"] = g_config.firmware.verify_sha256;
    firmware["update"] = g_config.firmware.getUpdateString();

    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["ssid"] = g_config.wifi.ssid;
    wifi["password"] = g_config.wifi.password;

    JsonObject api = doc.createNestedObject("api");
    api["key"] = g_config.api.key;
    api["basic_auth"] = g_config.api.basic_auth;

    doc["in_config_mode"] = g_config.in_config_mode;

    File f = LittleFS.open(PREF_FILE, "w");
    if (!f)
    {
        LOG_PRINT("Failed to open preferences file for writing");
        LittleFS.end();
        return false;
    }

    if (serializeJson(doc, f) == 0)
    {
        LOG_PRINT("Failed to write preferences json");
        f.close();
        LittleFS.end();
        return false;
    }

    f.close();
    LittleFS.end();
    return true;
#endif
}

static String buildPreferenceKey(const String &prefix, const String &key)
{
    return prefix + "_" + key;
}

void ConfigManager::appendMqttPreferences(const String &prefix, const MqttConfig &config)
{
#if defined(ESP32)
    if (config.host.isEmpty() || config.port == 0 || config.user.isEmpty() || config.password.isEmpty() || config.protocol.isEmpty())
    {
        LOG_PRINT("Incomplete MQTT config for prefix: " + prefix);
        return;
    }
    
    preferences.putString(buildPreferenceKey(prefix, PREF_SUFFIX_MQTT_HOST).c_str(), config.host);
    preferences.putInt(buildPreferenceKey(prefix, PREF_SUFFIX_MQTT_PORT).c_str(), config.port);
    preferences.putString(buildPreferenceKey(prefix, PREF_SUFFIX_MQTT_USER).c_str(), config.user);
    preferences.putString(buildPreferenceKey(prefix, PREF_SUFFIX_MQTT_PASSWORD).c_str(), config.password);
    preferences.putString(buildPreferenceKey(prefix, PREF_SUFFIX_MQTT_PROTOCOL).c_str(), config.protocol);
    preferences.putInt(buildPreferenceKey(prefix, PREF_SUFFIX_MQTT_TTL).c_str(), config.ttl);
#else
    (void)prefix;
    (void)config;
#endif
}

void ConfigManager::appendFirmwarePreferences(const FirmwareConfig &config)
{
#if defined(ESP32)
    preferences.putString(PREF_KEY_FW_BASE_URL, config.url);
    preferences.putString(PREF_KEY_FW_MANIFEST, config.manifest);
    preferences.putBool(PREF_KEY_FW_VERIFY_SHA, config.verify_sha256);

    preferences.putString(PREF_KEY_AUTO_UPDATE, config.getUpdateString());
#else
    (void)config;
#endif
}

void ConfigManager::appendWifiPreferences(const WifiConfig &config)
{
#if defined(ESP32)
    preferences.putString(PREF_KEY_WIFI_SSID, config.ssid);
    preferences.putString(PREF_KEY_WIFI_PASSWORD, config.password);
#else
    (void)config;
#endif
}

void ConfigManager::appendApiPreferences(const ApiConfig &config)
{
#if defined(ESP32)
    preferences.putString(PREF_KEY_DEVICE_API, config.key);
    preferences.putString(PREF_KEY_BASIC_AUTH, config.basic_auth);
#else
    (void)config;
#endif
}

bool ConfigManager::loadConfigFromPreferences()
{
#if defined(ESP32)
    if (!preferences.begin(PREF_NAMESPACE, true))
    {
        LOG_PRINT("Prefs begin failed");
        return false;
    }

    // MQTT Primary
    g_config.mqtt.primary.host = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_PRIMARY, PREF_SUFFIX_MQTT_HOST).c_str(), "");
    g_config.mqtt.primary.port = preferences.getInt(buildPreferenceKey(PREF_PREFIX_MQTT_PRIMARY, PREF_SUFFIX_MQTT_PORT).c_str(), 1883);
    g_config.mqtt.primary.user = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_PRIMARY, PREF_SUFFIX_MQTT_USER).c_str(), "");
    g_config.mqtt.primary.password = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_PRIMARY, PREF_SUFFIX_MQTT_PASSWORD).c_str(), "");
    g_config.mqtt.primary.protocol = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_PRIMARY, PREF_SUFFIX_MQTT_PROTOCOL).c_str(), "mqtt");
    g_config.mqtt.primary.ttl = preferences.getInt(buildPreferenceKey(PREF_PREFIX_MQTT_PRIMARY, PREF_SUFFIX_MQTT_TTL).c_str(), 0);
    // MQTT Secondary
    g_config.mqtt.secondary.host = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_SECONDARY, PREF_SUFFIX_MQTT_HOST).c_str(), "");
    g_config.mqtt.secondary.port = preferences.getInt(buildPreferenceKey(PREF_PREFIX_MQTT_SECONDARY, PREF_SUFFIX_MQTT_PORT).c_str(), 1883);
    g_config.mqtt.secondary.user = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_SECONDARY, PREF_SUFFIX_MQTT_USER).c_str(), "");
    g_config.mqtt.secondary.password = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_SECONDARY, PREF_SUFFIX_MQTT_PASSWORD).c_str(), "");
    g_config.mqtt.secondary.protocol = preferences.getString(buildPreferenceKey(PREF_PREFIX_MQTT_SECONDARY, PREF_SUFFIX_MQTT_PROTOCOL).c_str(), "mqtt");
    g_config.mqtt.secondary.ttl = preferences.getInt(buildPreferenceKey(PREF_PREFIX_MQTT_SECONDARY, PREF_SUFFIX_MQTT_TTL).c_str(), 0);
    // Firmware
    g_config.firmware.url = preferences.getString(PREF_KEY_FW_BASE_URL, "");
    g_config.firmware.manifest = preferences.getString(PREF_KEY_FW_MANIFEST, "");
    g_config.firmware.verify_sha256 = preferences.getBool(PREF_KEY_FW_VERIFY_SHA, true);
    g_config.firmware.setUpdate(preferences.getString(PREF_KEY_AUTO_UPDATE, "OTA").c_str());
    // WiFi
    g_config.wifi.ssid = preferences.getString(PREF_KEY_WIFI_SSID, "");
    g_config.wifi.password = preferences.getString(PREF_KEY_WIFI_PASSWORD, "");
    // API
    g_config.api.key = preferences.getString(PREF_KEY_DEVICE_API, "");
    g_config.api.basic_auth = preferences.getString(PREF_KEY_BASIC_AUTH, "");

    g_config.in_config_mode = preferences.getBool(PREF_KEY_IN_CONFIG_MODE, true); 

    preferences.end();
    return true;
#elif defined(ESP8266)
    if (!LittleFS.begin())
    {
        LOG_PRINT("LittleFS mount failed");
        return false;
    }

    if (!LittleFS.exists(PREF_FILE))
    {
        LOG_PRINT("Preferences file not found, using defaults");
        LittleFS.end();
        return false;
    }

    File f = LittleFS.open(PREF_FILE, "r");
    if (!f)
    {
        LOG_PRINT("Failed to open preferences file");
        LittleFS.end();
        return false;
    }

    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    LittleFS.end();

    if (err)
    {
        LOG_PRINT("Failed to parse preferences json: " + String(err.c_str()));
        return false;
    }

    JsonObject mqttPrimary = doc["mqtt_primary"];
    if (!mqttPrimary.isNull())
    {
        g_config.mqtt.primary.host = mqttPrimary["host"] | "";
        g_config.mqtt.primary.port = mqttPrimary["port"] | 1883;
        g_config.mqtt.primary.user = mqttPrimary["user"] | "";
        g_config.mqtt.primary.password = mqttPrimary["password"] | "";
        g_config.mqtt.primary.protocol = mqttPrimary["protocol"] | "mqtt";
        g_config.mqtt.primary.ttl = mqttPrimary["ttl"] | 0;
    }

    JsonObject mqttSecondary = doc["mqtt_secondary"];
    if (!mqttSecondary.isNull())
    {
        g_config.mqtt.secondary.host = mqttSecondary["host"] | "";
        g_config.mqtt.secondary.port = mqttSecondary["port"] | 1883;
        g_config.mqtt.secondary.user = mqttSecondary["user"] | "";
        g_config.mqtt.secondary.password = mqttSecondary["password"] | "";
        g_config.mqtt.secondary.protocol = mqttSecondary["protocol"] | "mqtt";
        g_config.mqtt.secondary.ttl = mqttSecondary["ttl"] | 0;
    }

    JsonObject firmware = doc["firmware"];
    if (!firmware.isNull())
    {
        g_config.firmware.url = firmware["url"] | "";
        g_config.firmware.manifest = firmware["manifest"] | "";
        g_config.firmware.verify_sha256 = firmware["verify_sha256"] | true;
        g_config.firmware.setUpdate((const char *)(firmware["update"] | "OTA"));
    }

    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull())
    {
        g_config.wifi.ssid = wifi["ssid"] | "";
        g_config.wifi.password = wifi["password"] | "";
    }

    JsonObject api = doc["api"];
    if (!api.isNull())
    {
        g_config.api.key = api["key"] | "";
        g_config.api.basic_auth = api["basic_auth"] | "";
    }

    g_config.in_config_mode = doc["in_config_mode"] | true;
    return true;
#endif
}

bool ConfigManager::loadConfig()
{
    loadConfigFromPreferences();

    if (settingsFetcher->fetchSettings(getDeviceId(), g_config))
    {
        LOG_INFO("Settings fetched successfully:");
        if (updateConfig(g_config))
        {
            LOG_INFO("Settings updated and saved to preferences.");
        }
        else
        {
            LOG_ERROR("Failed to update settings.");
        }
        delete settingsFetcher;
    }
    else
    {
        LOG_PRINT("Failed to fetch settings.");

        return false;
    }
    return true;
}

bool ConfigManager::setConfigValue(const String &key, const String &value)
{
    if (key == PREF_KEY_WIFI_SSID)
    {
        g_config.wifi.ssid = value;
    }
    else if (key == PREF_KEY_WIFI_PASSWORD)
    {
        g_config.wifi.password = value;
    }
    else if (key == PREF_KEY_DEVICE_API)
    {
        g_config.api.key = value;
    }
    else if (key == PREF_KEY_BASIC_AUTH)
    {
        g_config.api.basic_auth = value;
    }
    else
    {
        LOG_PRINT("Unknown config key: " + key);
        return false;
    }
    return true;
}

bool ConfigManager::finalizeConfigMode()
{
    g_config.in_config_mode = false;
    updateConfig(g_config);
    ESP.restart();
    return true;
}

bool ConfigManager::enterConfigMode()
{
    LOG_PRINT("Entering configuration mode...");
    g_config.in_config_mode = true;
    updateConfig(g_config);
    ESP.restart();
    return true;
}
