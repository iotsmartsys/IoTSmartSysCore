#pragma once

#include "Contracts/Settings/SettingsManager.h"

namespace iotsmartsys::core
{

    class ILogger;
    class ITimeProvider;

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
    };

} // namespace iotsmartsys::core