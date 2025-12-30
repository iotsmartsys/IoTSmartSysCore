#include "EspIdfCommandParser.h"

extern "C"
{
#include "cJSON.h"
}

namespace iotsmartsys::platform::espressif
{
    bool EspIdfCommandParser::parseCommand(const char *jsonPayload, size_t payloadLen, iotsmartsys::core::DeviceCommand &outCmd)
    {
        cJSON *root = cJSON_ParseWithLength(jsonPayload, payloadLen);
        if (!root)
        {
            return false;
        }

        cJSON *capabilityName = cJSON_GetObjectItem(root, "capability_name");
        cJSON *deviceId = cJSON_GetObjectItem(root, "device_id");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        cJSON *commandType = cJSON_GetObjectItem(root, "command_type");

        if (capabilityName && deviceId && value)
        {
            outCmd.capability_name = capabilityName->valuestring;
            outCmd.device_id = deviceId->valuestring;
            outCmd.value = value->valuestring;
            if (commandType)
                outCmd.command_type = commandType->valuestring;

            cJSON_Delete(root);
            return true;
        }

        cJSON_Delete(root);
        return false;
    }
}