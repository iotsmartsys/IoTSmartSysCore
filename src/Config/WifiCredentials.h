#pragma once

#include <cstddef>

namespace iotsmartsys::config
{
    struct WifiCredential
    {
        const char *ssid;
        const char *password;
    };
}

#if __has_include("Config/WifiCredentials.generated.h")
#include "Config/WifiCredentials.generated.h"
#else
namespace iotsmartsys::config
{
    static constexpr WifiCredential kWifiCredentials[1] = {{nullptr, nullptr}};
    static constexpr std::size_t kWifiCredentialCount = 0;
}
#endif

namespace iotsmartsys::config
{
    inline bool hasWifiCredentials()
    {
        return kWifiCredentialCount > 0 &&
               kWifiCredentials[0].ssid != nullptr &&
               kWifiCredentials[0].ssid[0] != '\0' &&
               kWifiCredentials[0].password != nullptr;
    }
}
