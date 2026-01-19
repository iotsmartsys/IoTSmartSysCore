#pragma once

#include "Contracts/Settings/SettingsGate.h"

namespace iotsmartsys::core::settings
{

    class SettingsGateImpl final : public ISettingsGate
    {
    public:
        SettingsGateImpl() = default;
        ~SettingsGateImpl() override = default;

        SettingsReadyLevel level() const override;

        void signalAvailable() override;
        void signalSynced() override;
        void signalSyncing() override;
        void signalError(iotsmartsys::core::common::StateResult err) override;
        void setLevel(SettingsReadyLevel level, iotsmartsys::core::common::StateResult lastErr) override;

        iotsmartsys::core::common::StateResult runWhenReady(
            SettingsReadyLevel want,
            SettingsGateCallback cb,
            void *user_ctx) override;

    private:
        struct Sub
        {
            bool used = false;
            SettingsReadyLevel want = SettingsReadyLevel::None;
            SettingsGateCallback cb = nullptr;
            void *ctx = nullptr;
        };

        static constexpr int kMaxSubs = 8;

        SettingsReadyLevel _level = SettingsReadyLevel::None;
        iotsmartsys::core::common::StateResult _last_err = iotsmartsys::core::common::StateResult::Ok;

        Sub _subs[kMaxSubs];

        static bool isAtLeast(SettingsReadyLevel have, SettingsReadyLevel want);
    };

} // namespace iotsmartsys::core::settings
