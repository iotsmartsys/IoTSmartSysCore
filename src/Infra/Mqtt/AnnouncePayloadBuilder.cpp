#include "Infra/Mqtt/AnnouncePayloadBuilder.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include "Infra/Wifi/WifiHelper.h"

AnnouncePayloadBuilder::AnnouncePayloadBuilder(const std::vector<Capability *> &capabilities, const std::vector<Property *> &properties)
    : capabilities(capabilities), properties(properties)
{
}

String AnnouncePayloadBuilder::buildCapabilitiesJson()
{
    String payload_capabilities = "";
    for (const auto &cap : capabilities)
    {
        LOG_PRINT("Capability: ");
        LOG_PRINTLN(cap->capability_name);

        payload_capabilities += "{\"capability_name\":\"" + cap->capability_name + "\",\"type\":\"" + cap->type + "\",\"value\":\"" + cap->value + "\"},";
    }

    if (payload_capabilities.length() > 0)
    {
        payload_capabilities = payload_capabilities.substring(0, payload_capabilities.length() - 1);
    }
    return String("[") + payload_capabilities + String("]");
}

String AnnouncePayloadBuilder::buildPropertiesJson()
{
    String props = "[";

    for (const auto &prop : properties)
    {
        props += "{\"name\":\"" + prop->property_name + "\",\"value\":\"" + prop->value + "\"}";
        if (prop != properties.back())
        {
            props += ",";
        }
    }

    props += "]";

    return props;
}

String AnnouncePayloadBuilder::buildDevicePayload()
{
    String ip_address = WiFi.localIP().toString();

    LOG_PRINTLN("Endereço IP: " + ip_address);

    String device_id = getDeviceId().c_str();
    String mac_address = getMacAddress().c_str();

    String capabilities_json = buildCapabilitiesJson();
    String properties_json = buildPropertiesJson();

    LOG_PRINTLN("Montando payload final do dispositivo...");
    String payload = "{\"device_id\":\"" + device_id + "\",\"ip_address\":\"" + ip_address + "\",\"mac_address\":\"" + mac_address + "\",\"properties\":" + properties_json + ",\"capabilities\":" + capabilities_json + "}";

    return payload;
}
