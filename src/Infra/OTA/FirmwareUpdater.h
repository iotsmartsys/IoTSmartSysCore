#pragma once

#include "Contracts/Settings/Settings.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"
#include "Models/ManifestInfo.h"
#include "IFirmwareManfiestParser.h"

using namespace iotsmartsys::core;

namespace iotsmartsys::ota
{
    enum class FirmwareUpdateCheckResult
    {
        NoUpdate,
        UpdateStarted,
        ManifestNotFound,
        ManifestFetchFailed
    };

    class FirmwareUpdater
    {
    public:
        FirmwareUpdater(ILogger &logger, IFirmwareManifestParser &manifestParser);

        FirmwareUpdateCheckResult checkAndUpdate(settings::FirmwareConfig currentSettings);
        bool hasCheckedForUpdate() const { return _updateHasChecked; }

    private:
        bool _updateHasChecked = false;
        ILogger &_logger;
        std::string _manifestUrl;
        std::string _baseUrl;
        bool _useHttps;
        bool _verifySha256;
        int _lastManifestHttpStatus{-1};
        IFirmwareManifestParser &_manifestParser;
        ManifestInfo fetchManifest();
        bool isRemoteNewer(const ManifestInfo &manifest);
        bool performOta(const ManifestInfo &manifest);
        void configure(const std::string &manifestUrl,
                       const std::string &baseUrl,
                       bool verifySha256 = false);
    };

} // namespace iotsmartsys::ota
