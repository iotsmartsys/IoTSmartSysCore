#include "DeviceIdentityProvider.h"

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

namespace iotsmartsys::platform::espressif::providers
{
    // Helper that fills mac[6] using Arduino WiFi API. Works on ESP32 and ESP8266.
    static void readMac(uint8_t mac[6])
    {
        // WiFi.macAddress(mac) works even if WiFi is not connected on these platforms.
        WiFi.macAddress(mac);
    }

    std::string DeviceIdentityProvider::getDeviceID() const
    {
#if defined(ARDUINO_ARCH_ESP32)
        const char *model = "esp32";
#elif defined(ARDUINO_ARCH_ESP8266)
        const char *model = "esp8266";
#else
        const char *model = "esp";
#endif

        uint8_t mac[6] = {0};
        readMac(mac);

        uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
        char buf[32];
        snprintf(buf, sizeof(buf), "%s-%06lX", model, (unsigned long)suffix);
        return std::string(buf);
    }

    std::string DeviceIdentityProvider::getDeviceUniqueId() const
    {
        uint8_t mac[6] = {0};
        readMac(mac);

        char buf[13];
        snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(buf);
    }

} // namespace iotsmartsys::platform::espressif::providers