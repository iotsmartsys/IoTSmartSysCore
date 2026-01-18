#include "App/Managers/DeviceStateManager.h"
#include "Contracts/Capabilities/ICapabilityType.h"

namespace iotsmartsys::app
{
    DeviceStateManager::DeviceStateManager(core::ILogger &logger, core::IHardwareAdapterFactory &factory, core::WiFiManager wifi, app::ProvisioningController provisioningController)
        : logger_(logger),
          factory_(factory),
          wifi_(wifi),
          provisioningController_(provisioningController)
    {
    }

    bool DeviceStateManager::configure(const LightConfig &cfg)
    {
        if (led)
        {
            logger_.warn("Status LED already configured. Returning existing instance.");
            return false;
        }

        void *adapterMem = malloc(factory_.outputAdapterSize());
        if (!adapterMem)
        {
            logger_.error("Failed to allocate memory for status LED hardware adapter.");
            return false;
        }

        led = factory_.createOutput(adapterMem, static_cast<std::uint8_t>(cfg.GPIO), cfg.highIsOn);
        led->setup();
        return true;
    }

    void DeviceStateManager::handle()
    {
        const uint32_t nowMs = millis();
        const bool provisioning = provisioningController_.isActive();

        if (provisioning)
        {
            state_ = core::StateDevice::InProvisioning;
            handleProvisioning(nowMs);
        }
        else
        {
            switch (wifi_.currentState())
            {
            case core::WiFiState::Connecting:
                state_ = core::StateDevice::ConnectingToNetwork;
                handleConnecting(nowMs);
                break;
            case core::WiFiState::Connected:
                state_ = core::StateDevice::Connected;
                handleIdle();
                break;
            default:
                // state_ = core::StateDevice::Error;
                handleIdle();
                break;
            }
        }
        if (led)
            led->handle();
    }

    void DeviceStateManager::handleProvisioning(uint32_t nowMs)
    {
        const uint32_t onMs = 100;
        const uint32_t offMs = 100;
        const uint32_t pauseMs = 1000;

        if (ledOn_)
        {
            if (nowMs - lastToggleMs_ >= onMs)
            {
                led->applyCommand(SWITCH_STATE_OFF);
                ledOn_ = false;
                lastToggleMs_ = nowMs;
                blinkCount_++;
            }
            return;
        }

        if (blinkCount_ >= 3)
        {
            if (nowMs - lastToggleMs_ >= pauseMs)
            {
                blinkCount_ = 0;
                lastToggleMs_ = nowMs;
            }
            return;
        }

        if (nowMs - lastToggleMs_ >= offMs)
        {
            led->applyCommand(SWITCH_STATE_ON);
            ledOn_ = true;
            lastToggleMs_ = nowMs;
        }
    }

    void DeviceStateManager::handleConnecting(uint32_t nowMs)
    {
        const uint32_t toggleMs = 250;
        if (nowMs - lastToggleMs_ < toggleMs)
        {
            return;
        }

        led->applyCommand(TOGGLE_COMMAND);
        lastToggleMs_ = nowMs;
    }

    void DeviceStateManager::handleIdle()
    {
        if (ledOn_)
        {
            return;
        }

        led->applyCommand(SWITCH_STATE_ON);
        ledOn_ = true;
    }

    void DeviceStateManager::handleError(uint32_t nowMs)
    {
        const uint32_t toggleMs = 100;
        if (nowMs - lastToggleMs_ < toggleMs)
        {
            return;
        }

        led->applyCommand(TOGGLE_COMMAND);
        lastToggleMs_ = nowMs;
    }
} // namespace iotsmartsys::app
