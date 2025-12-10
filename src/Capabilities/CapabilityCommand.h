#pragma once

#include "Infra/JsonHelpers/IoTJson.h"
#include <vector>
class CapabilityCommand
{
public:
    String capability_name;
    String value;
    String device_id;

    CapabilityCommand() {}

    bool fromJson(const char *json)
    {
        capability_name = getJsonValue(json, "capability_name");
        value = getJsonValue(json, "value");
        device_id = getJsonValue(json, "device_id");

        return true;
    }
};
