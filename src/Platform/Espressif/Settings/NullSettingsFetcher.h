#pragma once

#include "Contracts/Common/StateResult.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Settings/ISettingsFetcher.h"

namespace iotsmartsys::platform::espressif
{
    class NullSettingsFetcher final : public iotsmartsys::core::settings::ISettingsFetcher
    {
    public:
        explicit NullSettingsFetcher(iotsmartsys::core::ILogger &) {}

        iotsmartsys::core::common::StateResult start(const iotsmartsys::core::settings::SettingsFetchRequest &,
                                                     iotsmartsys::core::settings::SettingsFetchCallback cb,
                                                     void *user_ctx) override
        {
            if (cb)
            {
                iotsmartsys::core::settings::SettingsFetchResult res{};
                res.err = iotsmartsys::core::common::StateResult::NotSupported;
                res.http_status = -1;
                res.cancelled = false;
                cb(res, user_ctx);
            }
            return iotsmartsys::core::common::StateResult::NotSupported;
        }

        void cancel() override {}

        bool isRunning() const override { return false; }
    };
} // namespace iotsmartsys::platform::espressif
