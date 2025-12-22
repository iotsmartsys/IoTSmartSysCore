#pragma once
#include <string>

namespace iotsmartsys::core::settings
{
    struct WifiConfig
    {
        std::string ssid;
        std::string password;
        
        bool hasChanged(const WifiConfig &other) const
        {
            return (ssid != other.ssid || password != other.password);
        }
    };
} // namespace iotsmartsys::core::settings