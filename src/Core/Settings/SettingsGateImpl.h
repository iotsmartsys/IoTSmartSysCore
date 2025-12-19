#pragma once

#include "Contracts/Settings/SettingsGate.h"

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
}

namespace iotsmartsys::core::settings
{

    class SettingsGateImpl final : public ISettingsGate
    {
    public:
        SettingsGateImpl();
        ~SettingsGateImpl() override;

        SettingsReadyLevel level() const override;

        void signalAvailable() override;
        void signalSynced() override;
        void signalError(iotsmartsys::core::common::Error err) override;

        iotsmartsys::core::common::Error runWhenReady(
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

        mutable SemaphoreHandle_t _mutex = nullptr;
        SettingsReadyLevel _level = SettingsReadyLevel::None;
        iotsmartsys::core::common::Error _last_err = iotsmartsys::core::common::Error::Ok;

        Sub _subs[kMaxSubs];

        static bool isAtLeast(SettingsReadyLevel have, SettingsReadyLevel want);
        void lock() const;
        void unlock() const;

        void setLevel(SettingsReadyLevel newLevel);
    };

} // namespace iotsmartsys::core::settings