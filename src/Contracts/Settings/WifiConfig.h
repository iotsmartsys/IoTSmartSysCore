#pragma once
#include <string>

namespace iotsmartsys::core::settings
{
    struct WifiProfileConfig
    {
        std::string ssid;
        std::string password;

        bool isValid() const
        {
            return !ssid.empty() && !password.empty();
        }

        bool hasChanged(const WifiProfileConfig &other) const
        {
            return ssid != other.ssid || password != other.password;
        }
    };

    struct WifiConfig
    {
        // Legacy/current selected credentials. Kept as a mirror of the active profile
        // so existing telemetry and provisioning code keeps working.
        std::string ssid;
        std::string password;

        WifiProfileConfig primary;
        WifiProfileConfig secondary;
        WifiProfileConfig tertiary;
        std::string profile{"primary"};

        const WifiProfileConfig &selected() const
        {
            if (profile == "tertiary" && tertiary.isValid())
            {
                return tertiary;
            }
            if (profile == "secondary" && secondary.isValid())
            {
                return secondary;
            }
            return primary;
        }

        bool hasAnyProfile() const
        {
            return primary.isValid() || secondary.isValid() || tertiary.isValid();
        }

        bool isValid() const
        {
            return hasAnyProfile() || (!ssid.empty() && !password.empty());
        }

        void syncSelectedLegacyFields()
        {
            if (!hasAnyProfile())
            {
                if (!ssid.empty() || !password.empty())
                {
                    primary.ssid = ssid;
                    primary.password = password;
                    profile = "primary";
                }
                return;
            }

            const WifiProfileConfig &active = selected();
            ssid = active.ssid;
            password = active.password;
            if (profile.empty())
            {
                profile = "primary";
            }
        }

        bool hasChanged(const WifiConfig &other) const
        {
            return (ssid != other.ssid ||
                    password != other.password ||
                    primary.hasChanged(other.primary) ||
                    secondary.hasChanged(other.secondary) ||
                    tertiary.hasChanged(other.tertiary) ||
                    profile != other.profile);
        }
    };
} // namespace iotsmartsys::core::settings
