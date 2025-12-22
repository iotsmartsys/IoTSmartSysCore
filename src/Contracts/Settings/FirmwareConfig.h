#pragma once
#include <string>
#include <cstring>

namespace iotsmartsys::core::settings
{

    enum class FirmwareUpdateMethod
    {
        NONE,
        OTA,
        AUTO
    };

    struct FirmwareConfig
    {
        std::string url;
        std::string manifest;
        bool verify_sha256{false};
        FirmwareUpdateMethod update{FirmwareUpdateMethod::OTA};

        void setUpdate(const char *value)
        {
            update = FirmwareConfig::firmwareUpdateMethodFromCString(value);
        }

        std::string getUpdateString() const
        {
            switch (update)
            {
            case FirmwareUpdateMethod::NONE:
                return "NONE";
            case FirmwareUpdateMethod::OTA:
                return "OTA";
            case FirmwareUpdateMethod::AUTO:
                return "AUTO";
            default:
                return "NONE";
            }
        }

        static FirmwareUpdateMethod firmwareUpdateMethodFromCString(const char *value)
        {
            if (value == nullptr)
                return FirmwareUpdateMethod::NONE;

            // Comparação case-insensitive simples
            std::string v(value);
            if (strcasecmp(v.c_str(), "none") == 0)
                return FirmwareUpdateMethod::NONE;
            if (strcasecmp(v.c_str(), "ota") == 0)
                return FirmwareUpdateMethod::OTA;
            if (strcasecmp(v.c_str(), "auto") == 0)
                return FirmwareUpdateMethod::AUTO;

            return FirmwareUpdateMethod::NONE;
        }

        bool hasChanged(const FirmwareConfig &other) const
        {
            return (url != other.url || manifest != other.manifest ||
                    verify_sha256 != other.verify_sha256 ||
                    update != other.update);
        }
    };

} // namespace iotsmartsys::core::settings