#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)

#include "DeviceIdentityProvider.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

namespace iotsmartsys::platform::espressif::providers
{
    const char *chipModelStr()
    {
        return "ESP8266";
    }

    std::string DeviceIdentityProvider::getDeviceID() const
    {
        uint8_t mac[6] = {0};
        // Works even if WiFi is not connected.
        WiFi.macAddress(mac);

        const char *model = "esp8266";
        uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
        char buf[32];
        snprintf(buf, sizeof(buf), "%s-%06lX", model, (unsigned long)suffix);
        return std::string(buf);
    }

    std::string DeviceIdentityProvider::getDeviceUniqueId() const
    {
        uint8_t mac[6] = {0};
        WiFi.macAddress(mac);

        char buf[13];
        snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(buf);
    }

    std::string DeviceIdentityProvider::getDeviceModel() const
    {
        return "ESP8266";
    }

} // namespace iotsmartsys::core
#endif