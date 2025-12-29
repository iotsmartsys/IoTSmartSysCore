#pragma once

#include <vector>
#include <string.h>
#include "Contracts/Capabilities/ICapability.h"
#include "Core/Models/Property.h"
#include "Contracts/Connections/WiFiManager.h"
#include "Contracts/Settings/Settings.h"

#include "Contracts/Logging/ILogger.h"


using namespace iotsmartsys::core;

namespace iotsmartsys::app
{
    class AnnouncePayloadBuilder
    {
    public:
        explicit AnnouncePayloadBuilder(std::vector<ICapability> capabilities, ILogger &logger);

        AnnouncePayloadBuilder &withBroker(const char *borker);
        AnnouncePayloadBuilder &withDeviceId(const char *deviceId);
        AnnouncePayloadBuilder &withVersion(const char *version);
        AnnouncePayloadBuilder &withBuild(const char *build);
        AnnouncePayloadBuilder &withProperty(const Property &property);
        AnnouncePayloadBuilder &withIpAddress(const char *ipAddress);
        AnnouncePayloadBuilder &withMacAddress(const char *macAddress);

        std::string build();

    private:
        std::string buildCapabilitiesJson();
        std::string buildPropertiesJson();
        std::string buildDeviceInfoJson();

        std::vector<ICapability> _capabilities;
        std::vector<Property> properties;
        ILogger &_logger;
        
        std::string ipAddress_;
        std::string macAddress_;
        std::string deviceId_;
        std::string borker_;
        std::string version_;
        std::string build_;
    };
}
