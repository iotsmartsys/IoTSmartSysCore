#pragma once

#include <cstdint>
#include <cstdlib>

#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Settings/SettingsManager.h"
#include "Core/Commands/SystemCommandProcessor.h"
#include "App/Builders/Configs/CapabilityConfig.h"

namespace iotsmartsys::app
{
    class FactoryResetButtonController
    {
    public:
        FactoryResetButtonController(core::ILogger &logger,
                                     core::settings::SettingsManager &settingsManager,
                                     core::SystemCommandProcessor &systemCommandProcessor,
                                     core::IHardwareAdapterFactory &factory);

        void configure(PushButtonConfig cfg);
        void handle();

    private:
        void releaseButton();

        core::ILogger &logger_;
        core::settings::SettingsManager &settingsManager_;
        core::SystemCommandProcessor &systemCommandProcessor_;
        core::IHardwareAdapterFactory &factory_;
        core::IInputHardwareAdapter *button_{nullptr};
    };
} // namespace iotsmartsys::app
