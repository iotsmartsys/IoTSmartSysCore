#include <Arduino.h>
#include "WifiHelper.h"
#include <vector>
#include <string>
#include <algorithm>
#include "Infra/Utils/Logger.h"

void setupWifi(const char *ssid, const char *password)
{
    LOG_INFO("Conectando ao WiFi com as credenciais fornecidas...");
    LOG_PRINT("SSID: ");
    LOG_PRINTLN(ssid);

    if (!connectToWifi(ssid, password))
    {
        LOG_WARN("Falha ao conectar ao WiFi.");
    }
}

std::vector<std::string> getAvailableSSIDs()
{
    std::vector<std::string> ssids;
    int numSsid = WiFi.scanNetworks();
    for (int i = 0; i < numSsid; i++)
    {
        ssids.push_back(WiFi.SSID(i).c_str());
    }
    return ssids;
}

bool connectToWifi(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 30000;
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < wifiTimeout)
    {
        delay(500);
        LOG_PRINT(".");
    }
    return WiFi.status() == WL_CONNECTED;
}

void maintainWiFiConnection(const char *ssid, const char *password)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_WARN("Wi-Fi desconectado. Tentando reconectar...");
        connectToWifi(ssid, password);
    }
}
String getDeviceId()
{
#if defined(TRANSPORT_ESP_NOW)

#ifdef ESP32
    uint64_t mac = ESP.getEfuseMac();
    char buf[4 + 12 + 1];
    strcpy(buf, "esp-");
    snprintf(buf + 4, 13, "%02x%02x%02x%02x%02x%02x",
             (uint8_t)(mac >> 40), (uint8_t)(mac >> 32), (uint8_t)(mac >> 24),
             (uint8_t)(mac >> 16), (uint8_t)(mac >> 8), (uint8_t)(mac));
    return String(buf);
#else
    String macStr = WiFi.macAddress();
    macStr.toLowerCase();
    macStr.replace(":", "");
    return String("esp-") + macStr;
#endif
#else

#ifdef ESP32
    return WiFi.getHostname();
#else
    const char *hostname = WiFi.getHostname();
    String hostname_str = String(hostname);
    hostname_str.replace("ESP-", "esp8266-");
    return hostname_str;
#endif
#endif
}

/// @brief Get the MAC address of the device.
/// @return The MAC address as a String in format "38:F0:20:9E:9E:F0".
String getMacAddress()
{
#ifdef ESP32
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char str[18];
    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String(str);
#else
    String macStr = WiFi.macAddress();

    return macStr;
#endif
}
