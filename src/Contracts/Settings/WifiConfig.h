#pragma once
#include <string>

namespace iotsmartsys::core::settings
{
    struct WifiConfig
    {
        std::string ssid;
        std::string password;
    };
} // namespace iotsmartsys::core::settings