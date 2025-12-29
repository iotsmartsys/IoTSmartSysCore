#pragma once

#include "Infra/OTA/IFirmwareManfiestParser.h"

using namespace iotsmartsys::ota;

namespace iotsmartsys::platform::espressif::ota
{
    class EspIdFirmwareManifestParser : public IFirmwareManifestParser
    {
    public:
        ~EspIdFirmwareManifestParser() override = default;
        bool parseManifest(const char *jsonPayload,
                           size_t payloadLen,
                           iotsmartsys::ota::ManifestInfo &outInfo) override;
    };
} // namespace iotsmartsys::ota