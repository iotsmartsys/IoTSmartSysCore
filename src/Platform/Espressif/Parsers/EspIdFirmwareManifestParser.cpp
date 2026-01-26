#include "EspIdFirmwareManifestParser.h"

#include "Platform/Common/Json/JsonPathExtractor.h"

namespace iotsmartsys::platform::espressif::ota
{
    bool EspIdFirmwareManifestParser::parseManifest(const char *jsonPayload,
                                                    size_t payloadLen,
                                                    iotsmartsys::ota::ManifestInfo &outInfo)
    {
        if (!jsonPayload || payloadLen == 0)
            return false;

        iotsmartsys::platform::common::json::JsonPathExtractor ext(jsonPayload, payloadLen);

        std::string module, env, version, urlPath, checksumType, checksumValue;
        int sizeInt = 0;
        bool mandatory = false;

        if (!ext.getString("module", module)) return false;
        if (!ext.getString("env", env)) return false;
        if (!ext.getString("version", version)) return false;

        // accept both url_path and urlPath
        if (!ext.getString("url_path", urlPath))
        {
            (void)ext.getString("urlPath", urlPath);
        }
        if (urlPath.empty()) return false;

        // size: try integer
        if (!ext.getInt("size", sizeInt)) return false;

        // mandatory optional
        (void)ext.getBool("mandatory", mandatory);

        // checksum.type/value required under checksum
        if (!ext.getString("checksum.type", checksumType)) return false;
        if (!ext.getString("checksum.value", checksumValue)) return false;

        outInfo.module = module;
        outInfo.env = env;
        outInfo.version = version;
        outInfo.urlPath = urlPath;
        outInfo.size = static_cast<size_t>(sizeInt);
        outInfo.mandatory = mandatory;
        outInfo.checksumType = checksumType;
        outInfo.checksumValue = checksumValue;
        outInfo.valid = true;

        return true;
    }

} // namespace iotsmartsys::platform::espressif::ota
