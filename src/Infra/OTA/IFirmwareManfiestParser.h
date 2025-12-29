#pragma once

#include "Models/ManifestInfo.h"

namespace iotsmartsys::ota
{
    class IFirmwareManifestParser
    {
    public:
        virtual ~IFirmwareManifestParser() = default;

        virtual bool parseManifest(const char *jsonPayload,
                                   size_t payloadLen,
                                   iotsmartsys::ota::ManifestInfo &outInfo) = 0;
    };
} // namespace iotsmartsys::ota