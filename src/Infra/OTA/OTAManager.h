#pragma once

#include <Arduino.h>
#include "Contracts/Settings/IReadOnlySettingsProvider.h"
#include "Contracts/Logging/ILogger.h"
#include "FirmwareUpdater.h"
#include "OTA.h"

using namespace iotsmartsys::core;
using namespace iotsmartsys::core::settings;

namespace iotsmartsys::ota
{
    class OTAManager
    {
    public:
        OTAManager(IReadOnlySettingsProvider &settingsProvider, ILogger &logger, IFirmwareManifestParser &manifestParser, OTA &ota);

        void handle();

    private:
        IReadOnlySettingsProvider &_settingsProvider;
        FirmwareUpdater _firmwareUpdater;
        ILogger &_logger;
        OTA &_ota;

        void update(FirmwareConfig settings);
    };
}
