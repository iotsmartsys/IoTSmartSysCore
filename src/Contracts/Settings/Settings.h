#pragma once

#include "MqttSettings.h"
#include "FirmwareConfig.h"
#include "WifiConfig.h"
#include "ApiConfig.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::core::settings
{
    struct Settings
    {
        bool in_config_mode{true};

        const char *clientId;
        LogLevel logLevel{LogLevel::Error};
        const char *logLevelStr() const
        {
            switch (logLevel)
            {
            case LogLevel::Error:
                return "error";
            case LogLevel::Warn:
                return "warn";
            case LogLevel::Info:
                return "info";
            case LogLevel::Debug:
                return "debug";
            case LogLevel::Trace:
                return "trace";
            default:
                return "error";
            }
        }

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