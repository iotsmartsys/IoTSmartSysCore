
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

#include <esp_chip_info.h>

    const char *chipModelStr()
    {
        esp_chip_info_t info;
        esp_chip_info(&info);

        switch (info.model)
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

    std::string DeviceIdentityProvider::getDeviceID() const
    {
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
    }

    std::string DeviceIdentityProvider::getDeviceUniqueId() const
    {
        uint8_t mac[6] = {0};
        esp_efuse_mac_get_default(mac);

        char buf[13];
        snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        return std::string(buf);
    }

    std::string DeviceIdentityProvider::getDeviceModel() const
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        switch (chip_info.model)
        {

        case CHIP_ESP32:
            return "ESP32";
        case CHIP_ESP32S2:
            return "ESP32-S2";
        case CHIP_ESP32S3:
            return "ESP32-S3";
        case CHIP_ESP32C3:
            return "ESP32-C3";
        case CHIP_ESP32H2:
            return "ESP32-H2";
        default:
            return "ESP32-Unknown";
        }
    }

} // namespace iotsmartsys::core
#endif