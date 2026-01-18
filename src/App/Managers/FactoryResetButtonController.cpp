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

        void *mem = malloc(factory_.outputAdapterSize());
        if (!mem)
        {
            logger_.error("Failed to allocate memory for factory reset button adapter.");
            return;
        }

        button_ = factory_.createInput(mem, static_cast<std::uint8_t>(pin),
                                       activeLow ? core::HardwareDigitalLogic::LOW_IS_ON : core::HardwareDigitalLogic::HIGH_IS_ON,
                                       core::InputPullMode::PULL_UP);
        button_->setup();
    }

    void FactoryResetButtonController::handle()
    {
        if (!button_)
        {
            return;
        }

        button_->handle();
        if (button_->digitalActive() && millis() - button_->lastStateReadMillis() > 15000)
        {
            logger_.warn("Factory reset button pressed. Clearing settings and restarting...");
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

        factory_.outputAdapterDestructor()(button_);
        button_ = nullptr;
    }
} // namespace iotsmartsys::app
