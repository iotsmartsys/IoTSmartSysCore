#include "DeviceIdentityProvider.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
extern "C"
{
#include "esp_system.h"
#include "esp_efuse.h"
#include "esp_chip_info.h"
#include "esp_mac.h"
}
#elif defined(ARDUINO_ARCH_ESP8266)
#include <Arduino.h>
#include <ESP8266WiFi.h>
#endif

namespace iotsmartsys::platform::espressif::providers
{
    std::string DeviceIdentityProvider::getDeviceID() const
    {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        const char *model = "ESP";
        switch (chip_info.model)
        {
        case CHIP_ESP32:
            model = "esp32";
            break;
        case CHIP_ESP32S2:
            model = "esp32s2";
            break;
        case CHIP_ESP32S3:
            model = "esp32s3";
            break;
        case CHIP_ESP32C3:
            model = "esp32c3";
            break;
        case CHIP_ESP32H2:
            model = "esp32h2";
            break;
        default:
            model = "ESP";
            break;
        }

        uint8_t mac[6] = {0};
        esp_efuse_mac_get_default(mac);

        uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
        char buf[32];
        snprintf(buf, sizeof(buf), "%s-%06lX", model, (unsigned long)suffix);
        return std::string(buf);

#elif defined(ARDUINO_ARCH_ESP8266)
        uint8_t mac[6] = {0};
        // Works even if WiFi is not connected.
        WiFi.macAddress(mac);

        const char *model = "esp8266";
        uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
        char buf[32];
        snprintf(buf, sizeof(buf), "%s-%06lX", model, (unsigned long)suffix);
        return std::string(buf);

#else
        // Unknown platform
        return std::string("ESP");
#endif
    }

    std::string DeviceIdentityProvider::getDeviceUniqueId() const
    {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        uint8_t mac[6] = {0};
        esp_efuse_mac_get_default(mac);

        char buf[13];
        snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(buf);

#elif defined(ARDUINO_ARCH_ESP8266)
        uint8_t mac[6] = {0};
        WiFi.macAddress(mac);

        char buf[13];
        snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(buf);

#else
        return std::string();
#endif
    }
} // namespace iotsmartsys::core