#pragma once

#ifdef ESP8266
#include "Contracts/Settings/ISettingsFetcher.h"
#include "Contracts/Logging/ILogger.h"

#include <cstddef>
#include <cstdint>

namespace iotsmartsys::platform::esp8266
{
    class Esp8266SettingsFetcher final : public iotsmartsys::core::settings::ISettingsFetcher
    {
    public:
        Esp8266SettingsFetcher(iotsmartsys::core::ILogger &logger);
        ~Esp8266SettingsFetcher() override;

        iotsmartsys::core::common::StateResult start(const iotsmartsys::core::settings::SettingsFetchRequest &req,
                                                     iotsmartsys::core::settings::SettingsFetchCallback cb,
                                                     void *user_ctx) override;

        void cancel() override;
        bool isRunning() const override;

    private:
        iotsmartsys::core::ILogger &_logger;
        volatile bool _cancel{false};
        bool _running{false};

        // request state
        iotsmartsys::core::settings::SettingsFetchRequest _req{};
        iotsmartsys::core::settings::SettingsFetchCallback _cb{nullptr};
        void *_user_ctx{nullptr};

        // body buffer
        char *_body{nullptr};
        std::size_t _body_cap{0};
        std::size_t _body_len{0};

        void resetBody();
        bool appendBody(const char *data, std::size_t len);
        void finishAndCallback(iotsmartsys::core::common::StateResult err, int http_status, bool cancelled);
    };
} // namespace iotsmartsys::platform::esp8266

#endif // ESP8266