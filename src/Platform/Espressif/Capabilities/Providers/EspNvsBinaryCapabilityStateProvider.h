// Platform/Espressif/Capabilities/Providers/EspNvsBinaryCapabilityStateProvider.h
#pragma once

#ifdef ESP32

#include "Contracts/Providers/IBinaryCapabilityStateProvider.h"
#include "Contracts/Logging/Log.h"

extern "C"
{
#include "nvs.h"
#include "nvs_flash.h"
}

namespace iotsmartsys::platform::espressif
{
    // Espressif/NVS implementation of the binary-capability-state persistence
    // boundary. Uses its own NVS namespace, separate from device settings, and
    // keeps a compact versioned snapshot capped at 8 active records (the
    // runtime's capability limit). loadSnapshot() performs the single NVS data
    // read for the boot; tryGet() only ever touches the in-memory cache.
    class EspNvsBinaryCapabilityStateProvider final : public core::providers::IBinaryCapabilityStateProvider
    {
    public:
        EspNvsBinaryCapabilityStateProvider();
        ~EspNvsBinaryCapabilityStateProvider() override = default;

        iotsmartsys::core::common::StateResult loadSnapshot() override;
        bool tryGet(const char *capability_name, const char *type, bool &outIsOn) const override;
        iotsmartsys::core::common::StateResult save(const char *capability_name, const char *type, bool isOn) override;

    private:
        static constexpr const char *NVS_NAMESPACE = "iotbcs";
        static constexpr const char *NVS_KEY = "state";
        static constexpr std::uint32_t STORAGE_VERSION = 1;
        static constexpr std::size_t MAX_RECORDS = 8;
        static constexpr std::size_t NAME_LEN = 48;
        static constexpr std::size_t TYPE_LEN = 24;

        struct StoredRecord
        {
            char capability_name[NAME_LEN];
            char type[TYPE_LEN];
            std::uint8_t isOn;
            std::uint8_t used;
        };

        struct StoredSnapshot
        {
            std::uint32_t version;
            StoredRecord records[MAX_RECORDS];
        };

        StoredSnapshot _cache{};
        bool _cacheLoaded{false};

        int findRecordIndex(const char *capability_name, const char *type) const;
        static esp_err_t ensureNvsInit();
        static void copyField(char *dst, std::size_t dstSize, const char *src);
    };
} // namespace iotsmartsys::platform::espressif

#endif // ESP32
