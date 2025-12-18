#pragma once

#include <cstdint>
#include "Contracts/Common/Error.h"

namespace iotsmartsys::core::settings
{
    struct HttpHeader
    {
        const char *key;
        const char *value;
    };

    struct SettingsFetchRequest
    {
        const char *url;

        const HttpHeader *headers{nullptr};
        std::size_t headers_count{0};

        // timeouts
        std::uint32_t connect_timeout_ms{5000};
        std::uint32_t read_timeout_ms{8000};

        std::size_t max_body_bytes{8 * 1024};

        // retry/backoff
        std::uint8_t max_attempts{3};
        std::uint32_t backoff_base_ms{400}; // 400ms
        std::uint32_t backoff_max_ms{8000}; // 8s
        bool retry_on_http_4xx{false};
    };

    struct SettingsFetchResult
    {
        iotsmartsys::core::common::Error err{iotsmartsys::core::common::Error::Unknown}; // erro de transporte/timeout/etc.
        int http_status{-1};     // status HTTP (200, 401, 500...), -1 se não houve
        bool cancelled{false};

        const char *body{nullptr};
        std::size_t body_len{0};
    };

    using SettingsFetchCallback = void (*)(const SettingsFetchResult &result, void *user_ctx);

    class ISettingsFetcher
    {
    public:
        virtual ~ISettingsFetcher() = default;

    virtual iotsmartsys::core::common::Error start(const SettingsFetchRequest &req,
                SettingsFetchCallback cb,
                void *user_ctx) = 0;

        virtual void cancel() = 0;

        virtual bool isRunning() const = 0;
    };
} // namespace iotsmartsys::core::settings