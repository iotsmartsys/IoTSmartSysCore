#pragma once

#include <cstdint>
#include <cstdlib>

#include "App/Builders/Configs/CapabilityConfig.h"
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Logging/Log.h"
#include "Core/Models/DigitalLogic.h"
#include "Contracts/Connections/WiFiManager.h"
#include "App/Managers/ProvisioningController.h"
#include "Contracts/Common/StateDevice.h"

namespace iotsmartsys::app
{
    class DeviceStateManager
    {
    public:
        DeviceStateManager(core::ILogger &logger, core::IHardwareAdapterFactory &factory, core::WiFiManager wifi, app::ProvisioningController provisioningController);

        bool configure(const LightConfig &cfg);
        void handle();
        core::StateDevice currentState() const { return state_; }

    private:
        
        void handleProvisioning(uint32_t nowMs);
        void handleConnecting(uint32_t nowMs);
        void handleIdle();
        void handleError(uint32_t nowMs);

        core::ILogger &logger_;
        core::WiFiManager wifi_;
        core::IHardwareAdapterFactory &factory_;
        core::ICommandHardwareAdapter *led{nullptr};
        app::ProvisioningController provisioningController_;

        uint32_t lastToggleMs_{0};
        int blinkCount_{0};
        core::StateDevice state_;
        bool ledOn_{false};
    };
} // namespace iotsmartsys::app
