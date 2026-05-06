#include "App/Managers/FactoryResetButtonController.h"

#include <Arduino.h>

namespace iotsmartsys::app
{
    FactoryResetButtonController::FactoryResetButtonController(core::ILogger &logger,
                                                               core::settings::SettingsManager &settingsManager,
                                                               core::SystemCommandProcessor &systemCommandProcessor,
                                                               core::IHardwareAdapterFactory &factory)
        : logger_(logger),
          settingsManager_(settingsManager),
          systemCommandProcessor_(systemCommandProcessor),
          factory_(factory)
    {
    }

    void FactoryResetButtonController::configure(PushButtonConfig cfg)
    {
        const auto pin = cfg.GPIO;
        const auto activeLow = !cfg.highIsOn;

        releaseButton();

        void *mem = malloc(factory_.inputAdapterSize());
        if (!mem)
        {
            logger_.error("FactoryReset", "Failed to allocate memory for button adapter.");
            return;
        }

        button_ = factory_.createInput(mem, static_cast<std::uint8_t>(pin),
                                       activeLow ? core::HardwareDigitalLogic::LOW_IS_ON : core::HardwareDigitalLogic::HIGH_IS_ON,
                                       core::InputPullMode::PULL_UP);
        button_->setup();
        buttonWasActive_ = false;
        resetTriggered_ = false;
        logger_.info("FactoryReset", "Configured button on GPIO=%d active=%s hold=15000ms.",
                     pin,
                     activeLow ? "LOW" : "HIGH");
    }

    void FactoryResetButtonController::handle()
    {
        if (!button_)
        {
            return;
        }

        button_->handle();
        const bool active = button_->digitalActive();
        if (!active)
        {
            buttonWasActive_ = false;
            resetTriggered_ = false;
            return;
        }

        if (!buttonWasActive_)
        {
            buttonWasActive_ = true;
            logger_.warn("FactoryReset", "Button hold started. Keep pressed for 15s to clear settings.");
        }

        if (!resetTriggered_ && millis() - button_->lastStateReadMillis() > 15000)
        {
            resetTriggered_ = true;
            logger_.warn("FactoryReset", "Hold threshold reached. Clearing settings and restarting.");
            settingsManager_.clear();
            delay(2000);
            systemCommandProcessor_.restartSafely();
        }
    }

    void FactoryResetButtonController::releaseButton()
    {
        if (!button_)
        {
            return;
        }

        factory_.inputAdapterDestructor()(button_);
        free(button_);
        button_ = nullptr;
        buttonWasActive_ = false;
        resetTriggered_ = false;
    }
} // namespace iotsmartsys::app
