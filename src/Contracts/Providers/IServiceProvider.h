#pragma once

#include "Contracts/Settings/SettingsManager.h"
#include "Contracts/Connections/WiFiManager.h"

namespace iotsmartsys::core
{

    class ILogger;
    class ITimeProvider;
    class IRandomProvider;
    class ISystemControl;

    namespace firmware
    {
        class IFirmwareRuntimeInfo;
    }

    namespace settings
    {
        class IReadOnlySettingsProvider;
        class ISettingsGate;
    }

    class IServiceProvider
    {
    public:
        virtual ~IServiceProvider() = default;

        virtual ILogger *logger() const = 0;
        virtual ITimeProvider *time() const = 0;

        virtual settings::IReadOnlySettingsProvider *getSettingsProvider() const = 0;
        virtual settings::ISettingsGate *getSettingsGate() const = 0;

        virtual settings::SettingsManager *getSettingsManager() const = 0;
        virtual core::WiFiManager *getWiFiManager() const = 0;

        virtual firmware::IFirmwareRuntimeInfo *getFirmwareRuntimeInfo() const = 0;
        virtual IRandomProvider *getRandomProvider() const = 0;
        virtual ISystemControl *getSystemControl() const = 0;
    };

} // namespace iotsmartsys::core
