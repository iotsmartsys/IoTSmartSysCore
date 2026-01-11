#pragma once
#include <string>
#include <cstring>

namespace iotsmartsys::core::settings
{
    struct FirmwareConfig
    {
        std::string url;
        std::string manifest;
        bool verify_sha256{false};
        std::string update = "ota"; // "none", "ota", "auto"

        bool isValid() const
        {
            return !url.empty();
        }

        bool hasChanged(const FirmwareConfig &other) const
        {
            return (url != other.url || manifest != other.manifest ||
                    verify_sha256 != other.verify_sha256 ||
                    update != other.update);
        }
    };

} // namespace iotsmartsys::core::settings