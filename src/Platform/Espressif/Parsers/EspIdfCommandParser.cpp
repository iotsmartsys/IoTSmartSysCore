#include "EspIdfCommandParser.h"
#include "Contracts/Commands/CommandTypes.h"

extern "C"
{
#include "cJSON.h"
}

namespace iotsmartsys::platform::espressif
{
    EspIdfCommandParser::EspIdfCommandParser(iotsmartsys::core::ILogger &logger)
        : _logger(logger)
    {
    }

    iotsmartsys::core::DeviceCommand *EspIdfCommandParser::parseCommand(const char *jsonPayload, size_t payloadLen)
    {
        cJSON *root = cJSON_ParseWithLength(jsonPayload, payloadLen);
        if (!root)
        {
            _logger.error("Failed to parse JSON payload.");
            return nullptr;
        }
        cJSON *capabilityName = cJSON_GetObjectItem(root, "capability_name");
        cJSON *deviceId = cJSON_GetObjectItem(root, "device_id");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        cJSON *commandType = cJSON_GetObjectItem(root, "command_type");

        if (capabilityName && deviceId && value)
        {
            iotsmartsys::core::DeviceCommand *outCmd = new iotsmartsys::core::DeviceCommand();
            outCmd->capability_name = capabilityName->valuestring ? capabilityName->valuestring : "";
            outCmd->device_id = deviceId->valuestring ? deviceId->valuestring : "";
            outCmd->value = value->valuestring ? value->valuestring : "";
            outCmd->command_type = (commandType && commandType->valuestring) ? commandType->valuestring : iotsmartsys::core::CommandTypeStrings::CAPABILITY_STR;

            cJSON_Delete(root);
            return outCmd;
        }

        cJSON_Delete(root);
        return nullptr;
    }
}
