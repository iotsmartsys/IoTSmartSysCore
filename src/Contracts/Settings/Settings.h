#pragma once

#include "MqttSettings.h"
#include "FirmwareConfig.h"
#include "WifiConfig.h"
#include "ApiConfig.h"

namespace iotsmartsys::core::settings
{
    struct Settings
    {
        bool in_config_mode{true};

        MqttSettings mqtt;
        FirmwareConfig firmware;
        WifiConfig wifi;
        ApiConfig api;

        bool isValidWifiConfig() const
        {
            return (wifi.ssid.empty() == false && wifi.password.empty() == false);
        }
    };
} // namespace iotsmartsys::core::settings