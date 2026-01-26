// Only compile the ESP-IDF fetcher on ESP32 targets
#include "Config/BuildConfig.h"
#if defined(ESP32) && IOTSMARTSYS_SETTINGS_FETCH_ENABLED

#pragma once

#include "Contracts/Settings/ISettingsFetcher.h"
#include "Contracts/Logging/ILogger.h"

extern "C"
{
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
}

namespace iotsmartsys::platform::espressif
{
    class EspIdfSettingsFetcher final : public iotsmartsys::core::settings::ISettingsFetcher
    {
    public:
        EspIdfSettingsFetcher(iotsmartsys::core::ILogger &logger);
        ~EspIdfSettingsFetcher() override;

        iotsmartsys::core::common::StateResult start(const iotsmartsys::core::settings::SettingsFetchRequest &req,
                                                     iotsmartsys::core::settings::SettingsFetchCallback cb,
                                                     void *user_ctx) override;

        void cancel() override;
        bool isRunning() const override;

    private:
            iotsmartsys::core::ILogger &_logger;
        static void taskEntry(void *arg);

        void run(); // executa o fetch (com retries)
        esp_err_t performOnce(int &out_http_status);

        bool shouldRetry(esp_err_t err, int http_status, std::uint8_t attempt) const;
        std::uint32_t computeBackoffMs(std::uint8_t attempt) const;

        // estado
        SemaphoreHandle_t _mutex{nullptr};
        TaskHandle_t _task{nullptr};
        volatile bool _cancel{false};
        bool _running{false};

        iotsmartsys::core::settings::SettingsFetchRequest _req{};
        iotsmartsys::core::settings::SettingsFetchCallback _cb{nullptr};
        void *_user_ctx{nullptr};

        // buffer de resposta (alocado uma vez por start)
        char *_body{nullptr};
        std::size_t _body_cap{0};
        std::size_t _body_len{0};

        // http client handle
        esp_http_client_handle_t _client{nullptr};

        // evento http
        static esp_err_t httpEventHandler(esp_http_client_event_t *evt);
        esp_err_t onHttpEvent(esp_http_client_event_t *evt);

        void resetBody();
        bool appendBody(const char *data, int len);
        void finishAndCallback(iotsmartsys::core::common::StateResult err, int http_status, bool cancelled);
    };
} // namespace iotsmartsys::platform::espressif

#endif // ESP32
