#pragma once

#include <Arduino.h>
#include "Contracts/Settings/IReadOnlySettingsProvider.h"
#include "Contracts/Settings/SettingsGate.h"
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
        OTAManager(IReadOnlySettingsProvider &settingsProvider, ILogger &logger, IFirmwareManifestParser &manifestParser, 
            #ifdef OTA_ENABLED
            OTA &ota, 
            #endif
            iotsmartsys::core::settings::ISettingsGate &settingsGate);

        void handle();

    private:
        IReadOnlySettingsProvider &_settingsProvider;
        FirmwareUpdater _firmwareUpdater;
        ILogger &_logger;
#ifdef OTA_ENABLED
        OTA &_ota;
#endif
        iotsmartsys::core::settings::ISettingsGate &_settingsGate;
        bool _lastNetworkReady{false};
        bool _lastSettingsReady{false};

        void update(FirmwareConfig settings);
    };
}
