#pragma once

#include <vector>
#include "Contracts/Events/CapabilityStateChanged.h"
#include "Infra/Settings/VersionInfo.h"

namespace iotsmartsys::core
{

    class DeviceAnnouncement
    {
    public:
        std::string device_id;
        std::string device_name;
        std::string wifi_ssid;
        std::string wifi_signal;
        std::string ip_address;
        std::string version = IOTSMARTSYSCORE_VERSION;
        std::vector<CapabilityStateChanged> capabilities;

        const char *toJson()
        {
            std::string payload_capabilities = "";
            for (const auto &cap : capabilities)
            {
                if (payload_capabilities.length() > 0)
                    payload_capabilities += ",";
                payload_capabilities += "{\"capability_name\":\"" + cap.capability_name + "\",\"type\":\"" + cap.type + "\",\"value\":\"" + cap.value + "\"}";
            }
            std::string payload = "{\"device_id\":\"" + device_id + "\",\"version\":\"" + version + "\",\"ip_address\":\"" + ip_address + "\",\"wifi_ssid\":\"" + wifi_ssid + "\",\"wifi_signal\":\"" + wifi_signal + "\",\"capabilities\":[" + payload_capabilities + "]}";

            return payload.c_str();
        }
    };
} // namespace iotsmartsys::core