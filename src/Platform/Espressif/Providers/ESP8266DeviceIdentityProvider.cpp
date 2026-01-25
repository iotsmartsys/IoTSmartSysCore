#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)

#include "DeviceIdentityProvider.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

namespace iotsmartsys::platform::espressif::providers
{
    namespace
    {
        struct CachedIdentity
        {
            std::string device_id;
            std::string unique_id;
            std::string model_name;
        };

        const CachedIdentity &get_cached_identity()
        {
            static CachedIdentity cache;
            static bool initialized = false;
            if (!initialized)
            {
                uint8_t mac[6] = {0};
                // Works even if WiFi is not connected.
                WiFi.macAddress(mac);

                const char *model = "esp8266";
                uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
                char id_buf[32];
                snprintf(id_buf, sizeof(id_buf), "%s-%06lX", model, (unsigned long)suffix);
                cache.device_id = id_buf;

                char uid_buf[13];
                snprintf(uid_buf, sizeof(uid_buf), "%02X%02X%02X%02X%02X%02X",
                         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                cache.unique_id = uid_buf;

                cache.model_name = "ESP8266";
                initialized = true;
            }
            return cache;
        }
    } // namespace

    std::string DeviceIdentityProvider::getDeviceID() const
    {
        return get_cached_identity().device_id;
    }

    std::string DeviceIdentityProvider::getDeviceUniqueId() const
    {
        return get_cached_identity().unique_id;
    }

    std::string DeviceIdentityProvider::getDeviceModel() const
    {
        return get_cached_identity().model_name;
    }

} // namespace iotsmartsys::platform::espressif::providers
#endif
