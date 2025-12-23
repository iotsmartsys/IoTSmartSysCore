#pragma once

#include "Contracts/Capabilities/ICapability.h"
#include "Contracts/Settings/SettingsGate.h"
#include "Contracts/Settings/IReadOnlySettingsProvider.h"

namespace iotsmartsys::core
{

    class CapabilityManager
    {

    private:
        iotsmartsys::core::ICapability *const *items{nullptr};
        size_t count{0};
        iotsmartsys::core::ICapability *operator[](size_t i) const { return items[i]; }

        iotsmartsys::core::settings::ISettingsGate &_settingsGate;
        bool _settingsReady{false};
        bool _lastSettingsReady{false};
        static void onSettingsReadyThunk(iotsmartsys::core::settings::SettingsReadyLevel level, void *ctx);
        void onSettingsReady(iotsmartsys::core::settings::SettingsReadyLevel level);
        iotsmartsys::core::ILogger &_logger;
        iotsmartsys::core::settings::IReadOnlySettingsProvider &_settingsProvider;
        iotsmartsys::core::settings::Settings *_currentSettings;

    public:
        CapabilityManager(iotsmartsys::core::ICapability *const *items, size_t count,
                          iotsmartsys::core::settings::ISettingsGate &settingsGate,
                          iotsmartsys::core::ILogger &logger,
                          iotsmartsys::core::settings::IReadOnlySettingsProvider &settingsProvider);

        void setup();
        void handle();
        iotsmartsys::core::ICommandCapability *getCommandCapabilityByName(const char *name) const;
    };

}