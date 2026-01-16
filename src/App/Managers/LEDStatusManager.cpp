#include "App/Managers/LEDStatusManager.h"
#include "Contracts/Capabilities/ICapabilityType.h"

namespace iotsmartsys::app
{
    LEDStatusManager::LEDStatusManager(core::ILogger &logger, core::IHardwareAdapterFactory &factory)
        : logger_(logger),
          factory_(factory)
    {
    }

    bool LEDStatusManager::configure(const LightConfig &cfg)
    {
        if (statusLed_)
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

        statusLed_ = factory_.createOutput(adapterMem, static_cast<std::uint8_t>(cfg.GPIO), cfg.highIsOn);
        statusLed_->setup();
        return true;
    }

    void LEDStatusManager::update(uint32_t nowMs, bool provisioning, bool connecting)
    {
        if (!statusLed_)
        {
            return;
        }

        // Keep adapter internal timing/state updated.
        statusLed_->handle();

        LedMode desiredMode = LedMode::Idle;
        if (provisioning)
        {
            desiredMode = LedMode::Provisioning;
        }
        else if (connecting)
        {
            desiredMode = LedMode::Connecting;
        }

        if (desiredMode != mode_)
        {
            setMode(desiredMode, nowMs);
        }

        if (mode_ == LedMode::Provisioning)
        {
            handleProvisioning(nowMs);
        }
        else if (mode_ == LedMode::Connecting)
        {
            handleConnecting(nowMs);
        }
        else
        {
            handleIdle();
        }
    }

    void LEDStatusManager::setMode(LedMode mode, uint32_t nowMs)
    {
        mode_ = mode;
        lastToggleMs_ = nowMs;
        blinkCount_ = 0;
        ledOn_ = false;
        statusLed_->applyCommand(SWITCH_STATE_OFF);
    }

    void LEDStatusManager::handleProvisioning(uint32_t nowMs)
    {
        const uint32_t onMs = 100;
        const uint32_t offMs = 100;
        const uint32_t pauseMs = 1000;

        if (ledOn_)
        {
            if (nowMs - lastToggleMs_ >= onMs)
            {
                statusLed_->applyCommand(SWITCH_STATE_OFF);
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
            statusLed_->applyCommand(SWITCH_STATE_ON);
            ledOn_ = true;
            lastToggleMs_ = nowMs;
        }
    }

    void LEDStatusManager::handleConnecting(uint32_t nowMs)
    {
        const uint32_t toggleMs = 250;
        if (nowMs - lastToggleMs_ < toggleMs)
        {
            return;
        }

        if (ledOn_)
        {
            statusLed_->applyCommand(SWITCH_STATE_OFF);
            ledOn_ = false;
        }
        else
        {
            statusLed_->applyCommand(SWITCH_STATE_ON);
            ledOn_ = true;
        }
        lastToggleMs_ = nowMs;
    }

    void LEDStatusManager::handleIdle()
    {
        if (ledOn_)
        {
            return;
        }

        statusLed_->applyCommand(SWITCH_STATE_ON);
        ledOn_ = true;
    }
} // namespace iotsmartsys::app
