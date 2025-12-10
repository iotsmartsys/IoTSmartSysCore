#pragma once

#include <vector>
#include <WString.h>
#include "Capabilities/Capability.h"
#include "Core/Models/Property.h"

class AnnouncePayloadBuilder
{
public:
    AnnouncePayloadBuilder(const std::vector<Capability *> &capabilities, const std::vector<Property *> &properties);
    String buildDevicePayload();

private:
    String buildCapabilitiesJson();
    String buildPropertiesJson();

    const std::vector<Capability *> &capabilities;
    const std::vector<Property *> &properties;
};
