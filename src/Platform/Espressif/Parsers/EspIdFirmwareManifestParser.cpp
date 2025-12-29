#include "EspIdFirmwareManifestParser.h"

extern "C"
{
#include "cJSON.h"
}

namespace iotsmartsys::platform::espressif::ota
{
    bool EspIdFirmwareManifestParser::parseManifest(const char *jsonPayload,
                                                    size_t payloadLen,
                                                    iotsmartsys::ota::ManifestInfo &outInfo)
    {
        cJSON *root = cJSON_ParseWithLength(jsonPayload, payloadLen);
        if (!root)
        {
            return false;
        }

        cJSON *module = cJSON_GetObjectItem(root, "module");
        cJSON *env = cJSON_GetObjectItem(root, "env");
        cJSON *version = cJSON_GetObjectItem(root, "version");
        cJSON *urlPath = cJSON_GetObjectItem(root, "url_path");
        cJSON *size = cJSON_GetObjectItem(root, "size");
        cJSON *mandatory = cJSON_GetObjectItem(root, "mandatory");
        cJSON *checksumBlock = cJSON_GetObjectItem(root, "checksum");

        if (module && env && version && urlPath && size && mandatory && checksumBlock)
        {
            outInfo.module = module->valuestring;
            outInfo.env = env->valuestring;
            outInfo.version = version->valuestring;
            outInfo.urlPath = urlPath->valuestring;
            outInfo.size = static_cast<size_t>(size->valuedouble);
            outInfo.mandatory = (mandatory->valueint != 0);

            cJSON *checksumType = cJSON_GetObjectItem(checksumBlock, "type");
            cJSON *checksumValue = cJSON_GetObjectItem(checksumBlock, "value");

            if (checksumType && checksumValue)
            {
                outInfo.checksumType = checksumType->valuestring;
                outInfo.checksumValue = checksumValue->valuestring;

                outInfo.valid = true;
                cJSON_Delete(root);
                return true;
            }
        }

        cJSON_Delete(root);
        return false;
    }

} // namespace iotsmartsys::platform::espressif::ota