#include "Contracts/Capabilities/Managers/CapabilityManager.h"
#include "Contracts/Capabilities/ICommandCapability.h"

namespace iotsmartsys::core
{
    CapabilityManager::CapabilityManager(iotsmartsys::core::ICapability *const *items, size_t count,
                                         iotsmartsys::core::settings::ISettingsGate &settingsGate,
                                         iotsmartsys::core::ILogger &logger,
                                         iotsmartsys::core::settings::IReadOnlySettingsProvider &settingsProvider)
        : items(items), count(count), _settingsGate(settingsGate), _logger(logger), _settingsProvider(settingsProvider)
    {
        const auto gateSubErr = _settingsGate.runWhenReady(
            iotsmartsys::core::settings::SettingsReadyLevel::Available,
            &CapabilityManager::onSettingsReadyThunk,
            this);
    }

    void CapabilityManager::onSettingsReadyThunk(iotsmartsys::core::settings::SettingsReadyLevel level, void *ctx)
    {
        auto *self = static_cast<CapabilityManager *>(ctx);
        if (!self)
            return;
        self->onSettingsReady(level);
    }

    void CapabilityManager::onSettingsReady(iotsmartsys::core::settings::SettingsReadyLevel level)
    {
        (void)level;
        _settingsReady = true;
        iotsmartsys::core::settings::Settings settings;
        _settingsProvider.copyCurrent(settings);
        _currentSettings = &settings;
    }

    iotsmartsys::core::ICommandCapability *CapabilityManager::getCommandCapabilityByName(const char *name) const
    {
        for (size_t i = 0; i < count; ++i)
        {
            auto cap = items[i]->asCommandCapability();

            if (cap && cap->capability_name == name)
            {
                return cap;
            }
        }
        return nullptr;
    }

    void CapabilityManager::handle()
    {
        for (size_t i = 0; i < count; ++i)
        {
            items[i]->handle();
        }
    }

    void CapabilityManager::setup()
    {
        for (size_t i = 0; i < count; ++i)
        {
            items[i]->setup();
        }
    }

    std::vector<iotsmartsys::core::ICapability> CapabilityManager::getAllCapabilities() const
    {
        std::vector<iotsmartsys::core::ICapability> caps;
        for (size_t i = 0; i < count; ++i)
        {
            caps.push_back(*items[i]);
        }
        return caps;
    }
}