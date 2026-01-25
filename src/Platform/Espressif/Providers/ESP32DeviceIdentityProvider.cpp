
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

#include "DeviceIdentityProvider.h"
extern "C"
{
#include "esp_system.h"
#include "esp_efuse.h"
#include "esp_chip_info.h"
#include "esp_mac.h"
}

namespace iotsmartsys::platform::espressif::providers
{
    namespace
    {
        const char *model_name_from_chip(esp_chip_model_t model)
        {
            switch (model)
            {
            case CHIP_ESP32:
                return "ESP32";
#ifdef CHIP_ESP32S2
            case CHIP_ESP32S2:
                return "ESP32-S2";
#endif
#ifdef CHIP_ESP32S3
            case CHIP_ESP32S3:
                return "ESP32-S3";
#endif
#ifdef CHIP_ESP32C3
            case CHIP_ESP32C3:
                return "ESP32-C3";
#endif
#ifdef CHIP_ESP32C2
            case CHIP_ESP32C2:
                return "ESP32-C2";
#endif
#ifdef CHIP_ESP32C6
            case CHIP_ESP32C6:
                return "ESP32-C6";
#endif
#ifdef CHIP_ESP32H2
            case CHIP_ESP32H2:
                return "ESP32-H2";
#endif
#ifdef CHIP_ESP32P4
            case CHIP_ESP32P4:
                return "ESP32-P4";
#endif
            default:
                return "ESP32-Unknown";
            }
        }

        const char *model_prefix_from_chip(esp_chip_model_t model)
        {
            switch (model)
            {
            case CHIP_ESP32:
                return "esp32";
#ifdef CHIP_ESP32S2
            case CHIP_ESP32S2:
                return "esp32s2";
#endif
#ifdef CHIP_ESP32S3
            case CHIP_ESP32S3:
                return "esp32s3";
#endif
#ifdef CHIP_ESP32C3
            case CHIP_ESP32C3:
                return "esp32c3";
#endif
#ifdef CHIP_ESP32C2
            case CHIP_ESP32C2:
                return "esp32c2";
#endif
#ifdef CHIP_ESP32C6
            case CHIP_ESP32C6:
                return "esp32c6";
#endif
#ifdef CHIP_ESP32H2
            case CHIP_ESP32H2:
                return "esp32h2";
#endif
#ifdef CHIP_ESP32P4
            case CHIP_ESP32P4:
                return "esp32p4";
#endif
            default:
                return "esp";
            }
        }

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
                esp_chip_info_t chip_info;
                esp_chip_info(&chip_info);

                uint8_t mac[6] = {0};
                esp_efuse_mac_get_default(mac);

                char id_buf[32];
                uint32_t suffix = ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
                snprintf(id_buf, sizeof(id_buf), "%s-%06lX", model_prefix_from_chip(chip_info.model),
                         (unsigned long)suffix);
                cache.device_id = id_buf;

                char uid_buf[13];
                snprintf(uid_buf, sizeof(uid_buf), "%02X%02X%02X%02X%02X%02X",
                         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                cache.unique_id = uid_buf;

                cache.model_name = model_name_from_chip(chip_info.model);
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
