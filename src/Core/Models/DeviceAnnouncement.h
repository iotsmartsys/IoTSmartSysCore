#pragma once

#include <Arduino.h>
#include <vector>
#include "Capabilities/CapabilityState.h"
#include "Infra/Settings/VersionInfo.h"

class DeviceAnnouncement
{
public:
    String device_id;
    String device_name;
    String wifi_ssid;
    String wifi_signal;
    String ip_address;
    String version = IOTSMARTSYSCORE_VERSION;
    std::vector<CapabilityState> capabilities;

    DeviceAnnouncement() {}
    DeviceAnnouncement(String device_id, String device_name, String wifi_ssid, String wifi_signal, String ip_address, std::vector<CapabilityState> capabilities)
        : device_id(device_id), device_name(device_name), wifi_ssid(wifi_ssid), wifi_signal(wifi_signal), ip_address(ip_address), capabilities(capabilities) {}

    String toJson()
    {
        String payload_capabilities = "";
        for (const auto &cap : capabilities)
        {
            if (payload_capabilities.length() > 0) payload_capabilities += ",";
            payload_capabilities += "{\"capability_name\":\"" + cap.capability_name + "\",\"type\":\"" + cap.type + "\",\"value\":\"" + cap.value + "\"}";
        }
        
        if (device_name == nullptr)
        {
            device_name = device_id.c_str();
        }

        String payload = "{\"device_id\":\"" + String(device_id) + "\",\"version\":\"" + version + "\",\"ip_address\":\"" + ip_address + "\",\"name\":\"" + device_name + "\",\"wifi_ssid\":\"" + wifi_ssid + "\",\"wifi_signal\":\"" + wifi_signal + "\",\"capabilities\":[" + payload_capabilities + "]}";

        return payload;
    }
};
