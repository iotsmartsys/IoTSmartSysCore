#include "AnnouncePayloadBuilder.h"
#include <utility>

namespace iotsmartsys::app
{

    AnnouncePayloadBuilder::AnnouncePayloadBuilder(std::vector<ICapability> capabilities, ILogger &logger)
        : _capabilities(std::move(capabilities)), _logger(logger)
    {
        for (const auto &cap : _capabilities)
        {
            _logger.info("Capability: %s, Type: %s, Value: %s",
                         cap.capability_name.c_str(),
                         cap.type.c_str(),
                         cap.value.c_str());
        }
    }

    std::string AnnouncePayloadBuilder::build()
    {
        std::string payload = "{";

        // Add device info
        payload += buildDeviceInfoJson() + ",";

        // Add device properties
        payload += "\"properties\":[" + buildPropertiesJson() + "],";

        // Add capabilities
        payload += "\"capabilities\":[" + buildCapabilitiesJson() + "]";

        payload += "}";

        return payload;
    }

    std::string AnnouncePayloadBuilder::buildCapabilitiesJson()
    {
        std::string capabilitiesJson = "";
        for (const auto &cap : _capabilities)
        {
            if (!capabilitiesJson.empty())
            {
                capabilitiesJson += ",";
            }
            capabilitiesJson += "{";
            capabilitiesJson += "\"capability_name\":\"" + cap.capability_name + "\",";
            capabilitiesJson += "\"type\":\"" + cap.type + "\",";
            capabilitiesJson += "\"value\":\"" + cap.value + "\"";
            capabilitiesJson += "}";
        }
        return capabilitiesJson;
    }

    std::string AnnouncePayloadBuilder::buildPropertiesJson()
    {
        std::string propertiesJson = "";

        for (const auto &prop : properties)
        {
            if (!propertiesJson.empty())
            {
                propertiesJson += ",";
            }
            propertiesJson += "{";
            propertiesJson += "\"name\":\"" + std::string(prop.property_name) + "\",";
            propertiesJson += "\"value\":\"" + std::string(prop.value) + "\"";
            propertiesJson += "}";
        }
        return propertiesJson;
    }

    std::string AnnouncePayloadBuilder::buildDeviceInfoJson()
    {
        std::string deviceInfoJson = "";
        deviceInfoJson += "\"device_id\":\"" + deviceId + "\",";
        deviceInfoJson += "\"ip_address\":\"" + std::string(ipAddress) + "\",";
        deviceInfoJson += "\"mac_address\":\"" + std::string(macAddress) + "\"";
        return deviceInfoJson;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withIpAddress(const char *ipAddress)
    {
        this->ipAddress = std::string(ipAddress);
        return *this;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withMacAddress(const char *macAddress)
    {
        this->macAddress = std::string(macAddress);
        return *this;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withBroker(const char *borker)
    {
        this->borker = std::string(borker);
        return *this;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withDeviceId(const char *deviceId)
    {
        this->deviceId = std::string(deviceId);
        return *this;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withVersion(const char *version)
    {
        this->version = std::string(version);
        return *this;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withBuild(const char *build)
    {
        this->build_ = std::string(build);
        return *this;
    }

    AnnouncePayloadBuilder &AnnouncePayloadBuilder::withProperty(const Property &property)
    {
        for (const auto &prop : properties)
        {
            if (prop.property_name == property.property_name)
            {
                return *this;
            }
        }

        this->properties.push_back(property);
        return *this;
    }

};
