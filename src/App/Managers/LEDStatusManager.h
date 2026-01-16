#pragma once

#include <cstdint>
#include <cstdlib>

#include "App/Builders/Configs/CapabilityConfig.h"
#include "Contracts/Adapters/ICommandHardwareAdapter.h"
#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Logging/Log.h"
#include "Core/Models/DigitalLogic.h"

namespace iotsmartsys::app
{
    class LEDStatusManager
    {
    public:
        LEDStatusManager(core::ILogger &logger, core::IHardwareAdapterFactory &factory);

        bool configure(const LightConfig &cfg);
        void update(uint32_t nowMs, bool provisioning, bool connecting);

    private:
        enum class LedMode : uint8_t
        {
            Idle = 0,
            Provisioning = 1,
            Connecting = 2
        };

        void setMode(LedMode mode, uint32_t nowMs);
        void handleProvisioning(uint32_t nowMs);
        void handleConnecting(uint32_t nowMs);
        void handleIdle();

        core::ILogger &logger_;
        core::IHardwareAdapterFactory &factory_;
        core::ICommandHardwareAdapter *statusLed_{nullptr};

        uint32_t lastToggleMs_{0};
        int blinkCount_{0};
        LedMode mode_{LedMode::Idle};
        bool ledOn_{false};
    };
} // namespace iotsmartsys::app
