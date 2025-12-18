#include <Arduino.h>
#include <unity.h>

extern "C" {
#include "nvs_flash.h"
#include "esp_err.h"
}

#include "Platform/Espressif/Settings/EspIdfSettingsParser.h"
#include "Platform/Espressif/Settings/Providers/EspIdfNvsSettingsProvider.h"
#include "Contracts/Settings/Settings.h"

using iotsmartsys::core::settings::Settings;
using iotsmartsys::platform::espressif::EspIdfSettingsParser;
using iotsmartsys::platform::espressif::EspIdfNvsSettingsProvider;

static const char* kJson = R"json(
{
  "mqtt": {
    "primary": {
      "host": "192.168.0.222",
      "port": 1883,
      "user": "0bb1a826899f4e26a51ea006e0a3419a",
      "password": "J&87huh*iow65",
      "protocol": "mqtt"
    },
    "secondary": {
      "host": "5edc7281737f43669750c30c332654b9.s1.eu.hivemq.cloud",
      "port": 8883,
      "user": "marceloc",
      "password": "Ma522770",
      "protocol": "mqtts",
      "ttl": 30
    },
    "topic": {
      "announce": "smarthome/discovery",
      "command": "device/{device_id}/command",
      "notify": "device/state"
    }
  },
  "prefix_auto_format_properies_json": "mqtt,firmware",
  "firmware": {
    "update": "auto",
    "url": "https://iotsmartsys.tailb602a3.ts.net/s3",
    "verifysha256": true,
    "manifest": "/firmwares/SmartHome-Firmwares/glp_meter_esp32/latest.json"
  }
}
)json";

void setUp(void)
{
  // Limpa NVS para o teste ser determinístico
  // (sem depender do que estava salvo antes)
  nvs_flash_erase();
  TEST_ASSERT_EQUAL(ESP_OK, nvs_flash_init());
}

void tearDown(void) {}

void test_settings_parse_save_load_roundtrip()
{
    EspIdfSettingsParser parser;
    EspIdfNvsSettingsProvider provider;

    // 1) Parse
    Settings parsed;
    const esp_err_t perr = parser.parse(kJson, parsed);
    TEST_ASSERT_EQUAL(ESP_OK, perr);

    // 2) Valida parse (campos críticos)
    TEST_ASSERT_EQUAL_STRING("192.168.0.222", parsed.mqtt.primary.host.c_str());
    TEST_ASSERT_EQUAL_INT(1883, parsed.mqtt.primary.port);
    TEST_ASSERT_EQUAL_STRING("0bb1a826899f4e26a51ea006e0a3419a", parsed.mqtt.primary.user.c_str());
    TEST_ASSERT_EQUAL_STRING("J&87huh*iow65", parsed.mqtt.primary.password.c_str());
    TEST_ASSERT_EQUAL_STRING("mqtt", parsed.mqtt.primary.protocol.c_str());

    TEST_ASSERT_EQUAL_STRING("5edc7281737f43669750c30c332654b9.s1.eu.hivemq.cloud", parsed.mqtt.secondary.host.c_str());
    TEST_ASSERT_EQUAL_INT(8883, parsed.mqtt.secondary.port);
    TEST_ASSERT_EQUAL_STRING("marceloc", parsed.mqtt.secondary.user.c_str());
    TEST_ASSERT_EQUAL_STRING("Ma522770", parsed.mqtt.secondary.password.c_str());
    TEST_ASSERT_EQUAL_STRING("mqtts", parsed.mqtt.secondary.protocol.c_str());
    TEST_ASSERT_EQUAL_INT(30, parsed.mqtt.secondary.ttl);

    TEST_ASSERT_EQUAL_STRING("smarthome/discovery", parsed.mqtt.announce_topic.c_str());
    TEST_ASSERT_EQUAL_STRING("device/{device_id}/command", parsed.mqtt.command_topic.c_str());
    TEST_ASSERT_EQUAL_STRING("device/state", parsed.mqtt.notify_topic.c_str());

    TEST_ASSERT_EQUAL_STRING("https://iotsmartsys.tailb602a3.ts.net/s3", parsed.firmware.url.c_str());
    TEST_ASSERT_EQUAL_STRING("/firmwares/SmartHome-Firmwares/glp_meter_esp32/latest.json", parsed.firmware.manifest.c_str());
    TEST_ASSERT_TRUE(parsed.firmware.verify_sha256);
    TEST_ASSERT_EQUAL_STRING("AUTO", parsed.firmware.getUpdateString().c_str()); // do seu model

    // 3) Save
    TEST_ASSERT_EQUAL(ESP_OK, provider.save(parsed));
    TEST_ASSERT_TRUE(provider.exists());

    // 4) Load
    Settings loaded;
    const esp_err_t lerr = provider.load(loaded);
    TEST_ASSERT_EQUAL(ESP_OK, lerr);

    // 5) Compara alguns campos (roundtrip)
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.primary.host.c_str(), loaded.mqtt.primary.host.c_str());
    TEST_ASSERT_EQUAL_INT(parsed.mqtt.primary.port, loaded.mqtt.primary.port);
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.primary.user.c_str(), loaded.mqtt.primary.user.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.primary.password.c_str(), loaded.mqtt.primary.password.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.primary.protocol.c_str(), loaded.mqtt.primary.protocol.c_str());

    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.secondary.host.c_str(), loaded.mqtt.secondary.host.c_str());
    TEST_ASSERT_EQUAL_INT(parsed.mqtt.secondary.port, loaded.mqtt.secondary.port);
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.secondary.user.c_str(), loaded.mqtt.secondary.user.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.secondary.password.c_str(), loaded.mqtt.secondary.password.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.secondary.protocol.c_str(), loaded.mqtt.secondary.protocol.c_str());
    TEST_ASSERT_EQUAL_INT(parsed.mqtt.secondary.ttl, loaded.mqtt.secondary.ttl);

    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.announce_topic.c_str(), loaded.mqtt.announce_topic.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.command_topic.c_str(), loaded.mqtt.command_topic.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.mqtt.notify_topic.c_str(), loaded.mqtt.notify_topic.c_str());

    TEST_ASSERT_EQUAL_STRING(parsed.firmware.url.c_str(), loaded.firmware.url.c_str());
    TEST_ASSERT_EQUAL_STRING(parsed.firmware.manifest.c_str(), loaded.firmware.manifest.c_str());
    TEST_ASSERT_EQUAL(parsed.firmware.verify_sha256, loaded.firmware.verify_sha256);
    TEST_ASSERT_EQUAL_STRING(parsed.firmware.getUpdateString().c_str(), loaded.firmware.getUpdateString().c_str());

    // 6) Erase
    TEST_ASSERT_EQUAL(ESP_OK, provider.erase());
    TEST_ASSERT_FALSE(provider.exists());
}

void setup()
{
  delay(200);
  UNITY_BEGIN();
  RUN_TEST(test_settings_parse_save_load_roundtrip);
  UNITY_END();
}

void loop() {}