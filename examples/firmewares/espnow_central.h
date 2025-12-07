#include <Arduino.h>
#include "Core/IoTCore.h"
#include "Utils/Logger.h"
#include "Transports/BridgeHooks.h"
#ifdef ESP_NOW_ENABLED
#include "esp_now_utils/esp_now_utils.h"
#endif

IoTCore *iotCore = new IoTCore();
#define INTERVAL_SENSOR_UPDATE 30
#define TEMPERATURE_SENSOR_PIN 4
#ifdef DHT_ENABLED
DHT dht(TEMPERATURE_SENSOR_PIN, DHT11);
#endif

#ifdef ESP_NOW_ENABLED
static bool parseMac(const char *str, uint8_t mac[6]) {
  if (!str) return false;
  int v[6];
  if (sscanf(str, "%x:%x:%x:%x:%x:%x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) == 6) {
    for (int i = 0; i < 6; ++i) mac[i] = (uint8_t)v[i];
    return true;
  }
  return false;
}
#endif

void mqtt_to_espnow_forward(const CapabilityCommand &cmd)
{
#ifdef ESP_NOW_ENABLED
  struct_message m{};
  strncpy(m.device_id, cmd.device_id.c_str(), sizeof(m.device_id) - 1);
  strncpy(m.capability_name, cmd.capability_name.c_str(), sizeof(m.capability_name) - 1);
  strncpy(m.value, cmd.value.c_str(), sizeof(m.value) - 1);
  m.type[0] = '\0';

  int targetIdx = -1;
  for (int i = 0; i < peerCount; ++i) {
    if (strncmp(peerDeviceIds[i], m.device_id, sizeof(peerDeviceIds[i])) == 0) {
      targetIdx = i; break;
    }
  }

  if (targetIdx >= 0) {
    esp_now_send(peerMacs[targetIdx], reinterpret_cast<const uint8_t *>(&m), sizeof(m));
  } else {
    static uint8_t broadcastAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    esp_now_send(broadcastAddress, reinterpret_cast<const uint8_t *>(&m), sizeof(m));
  }
#else
  (void)cmd;
#endif
}

void setup()
{
  LOG_BEGIN(115200);
  delay(2000);
  LOG_PRINTLN("[ESPNOW-CENTRAL] Setup");

#ifdef DS18B20_ENABLED
  iotCore->capabilityBuilder->addTemperatureSensorCapability(TEMPERATURE_SENSOR_PIN, INTERVAL_SENSOR_UPDATE);
#elif DHT_ENABLED
  dht.begin();
  iotCore->addHumiditySensorCapability(&dht, INTERVAL_SENSOR_UPDATE);
  iotCore->addTemperatureSensorCapability(&dht, INTERVAL_SENSOR_UPDATE);
#endif

  iotCore->setup();

#ifdef ESP_NOW_ENABLED
#ifdef ESPNOW_KNOWN_CLIENT_MAC
  {
    uint8_t mac[6];
    if (parseMac(ESPNOW_KNOWN_CLIENT_MAC, mac)) {
      esp_now_peer_info_t p = {};
      memcpy(p.peer_addr, mac, 6);
      p.channel = WiFi.channel();
      p.encrypt = false;
#ifdef ESP32
      p.ifidx = WIFI_IF_STA;
#endif
      if (esp_now_add_peer(&p) == ESP_OK) {
        LOG_PRINT("[ESPNOW-CENTRAL] Peer client adicionado: ");
        LOG_PRINTLN(ESPNOW_KNOWN_CLIENT_MAC);
      } else {
        LOG_PRINTLN("[ESPNOW-CENTRAL] Falha ao adicionar peer client");
      }
    }
  }
#endif
#endif
}

void loop()
{
  iotCore->handle();
}
