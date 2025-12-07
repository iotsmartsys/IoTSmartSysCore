#pragma once

// Namespace used when opening Preferences
#define PREF_NAMESPACE "device_cfg"

// Common keys
#define PREF_KEY_AUTO_UPDATE "auto_update"
#define PREF_KEY_IN_CONFIG_MODE "in_config_mode"

// MQTT prefixes and suffixes
#define PREF_PREFIX_MQTT_PRIMARY "mqtt_1"
#define PREF_PREFIX_MQTT_SECONDARY "mqtt_2"
#define PREF_SUFFIX_MQTT_HOST "host"
#define PREF_SUFFIX_MQTT_PORT "port"
#define PREF_SUFFIX_MQTT_USER "user"
#define PREF_SUFFIX_MQTT_PASSWORD "password"
#define PREF_SUFFIX_MQTT_PROTOCOL "protocol"
#define PREF_SUFFIX_MQTT_TTL "ttl"

// Firmware
#define PREF_KEY_FW_BASE_URL "fw_base_url"
#define PREF_KEY_FW_MANIFEST "manifest_url"
#define PREF_KEY_FW_VERIFY_SHA "verify_sha"

// WiFi
#define PREF_KEY_WIFI_SSID "wifi_ssid"
#define PREF_KEY_WIFI_PASSWORD "wifi_password"

// API
#define PREF_KEY_DEVICE_API "device_api_key"
#define PREF_KEY_BASIC_AUTH "basic_auth"