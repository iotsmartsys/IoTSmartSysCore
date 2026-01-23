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
        bool in_config_mode{false};

        const char *clientId;
        LogLevel logLevel{LogLevel::Info};
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
        bool _is_changed{false};

        void applyChanges(const Settings &other)
        {
            if (in_config_mode != other.in_config_mode)
            {
                in_config_mode = other.in_config_mode;
                _is_changed = true;
            }
            if (logLevel != other.logLevel)
            {
                logLevel = other.logLevel;
                _is_changed = true;
            }
            if (api.hasChanged(other.api) && other.api.isValid())
            {
                api = other.api;
                _is_changed = true;
            }
            if (firmware.hasChanged(other.firmware) && other.firmware.isValid())
            {
                firmware = other.firmware;
                _is_changed = true;
            }
            if (mqtt.hasChanged(other.mqtt) && other.mqtt.isValid())
            {
                mqtt = other.mqtt;
                _is_changed = true;
            }
            if (wifi.hasChanged(other.wifi) && other.isValidWifiConfig())
            {
                wifi = other.wifi;
                _is_changed = true;
            }
        }

        bool hasChanges() const
        {
            return _is_changed;
        }

        bool isValidWifiConfig() const
        {
            return (wifi.ssid.empty() == false && wifi.password.empty() == false);
        }

        bool isValidApiConfig() const
        {
            return api.isValid();
        }
    };
} // namespace iotsmartsys::core::settings
