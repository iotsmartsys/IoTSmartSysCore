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
    };
} // namespace iotsmartsys::core::settings