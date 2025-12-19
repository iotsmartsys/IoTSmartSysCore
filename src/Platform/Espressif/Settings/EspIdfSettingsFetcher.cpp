#include "Platform/Espressif/Settings/EspIdfSettingsFetcher.h"
#include "Contracts/Common/StateResult.h"

#include <cstring>
#include <algorithm>
#include "Infra/Certs/LetsEncryptISRGRootX1.h"

extern "C"
{
#include "esp_log.h"
#include "esp_random.h"
#include "esp_crt_bundle.h"
}

namespace iotsmartsys::platform::espressif
{
    using namespace iotsmartsys::core::settings;
    using iotsmartsys::core::common::StateResult;
    static const char *TAG = "SettingsFetcher";

    static StateResult map_esp_err(esp_err_t e)
    {
        switch (e)
        {
        case ESP_OK:
            return StateResult::Ok;
        case ESP_ERR_NO_MEM:
            return StateResult::NoMem;
        case ESP_ERR_INVALID_ARG:
            return StateResult::InvalidArg;
        case ESP_ERR_INVALID_STATE:
            return StateResult::InvalidState;
        case ESP_ERR_TIMEOUT:
            return StateResult::Timeout;
        default:
            return StateResult::IoError;
        }
    }

    EspIdfSettingsFetcher::EspIdfSettingsFetcher()
    {
        _mutex = xSemaphoreCreateMutex();
    }

    EspIdfSettingsFetcher::~EspIdfSettingsFetcher()
    {
        cancel();
        // espera task encerrar (sem travar: só se existir)
        for (int i = 0; i < 50; ++i)
        {
            if (!isRunning())
                break;
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        if (_client)
        {
            esp_http_client_cleanup(_client);
            _client = nullptr;
        }

        if (_body)
        {
            free(_body);
            _body = nullptr;
        }

        if (_mutex)
        {
            vSemaphoreDelete(_mutex);
            _mutex = nullptr;
        }
    }

    bool EspIdfSettingsFetcher::isRunning() const
    {
        return _running;
    }

    void EspIdfSettingsFetcher::cancel()
    {
        _cancel = true;
    }

    iotsmartsys::core::common::StateResult EspIdfSettingsFetcher::start(const SettingsFetchRequest &req,
                                                                  SettingsFetchCallback cb,
                                                                  void *user_ctx)
    {
        if (!cb || !req.url || req.url[0] == '\0')
            return StateResult::InvalidArg;

        xSemaphoreTake(_mutex, portMAX_DELAY);
        if (_running)
        {
            xSemaphoreGive(_mutex);
            return StateResult::InvalidState; // já rodando
        }

        _req = req;
        _cb = cb;
        _user_ctx = user_ctx;
        _cancel = false;

        // prepara buffer (cap = max_body_bytes + 1 p/ '\0')
        if (_body)
        {
            free(_body);
            _body = nullptr;
        }
        _body_cap = std::max<std::size_t>(req.max_body_bytes, 256);
        _body = (char *)malloc(_body_cap + 1);
        if (!_body)
        {
            xSemaphoreGive(_mutex);
            return StateResult::NoMem;
        }
        resetBody();

        // cria task dedicada (não trava firmware)
        _running = true;
        BaseType_t ok = xTaskCreate(&EspIdfSettingsFetcher::taskEntry,
                                    "settings_fetch",
                                    6144, // stack
                                    this,
                                    5, // prioridade
                                    &_task);

        if (ok != pdPASS)
        {
            _running = false;
            _task = nullptr;
            free(_body);
            _body = nullptr;
            xSemaphoreGive(_mutex);
            return StateResult::NoMem;
        }

        xSemaphoreGive(_mutex);
        return StateResult::Ok;
    }

    void EspIdfSettingsFetcher::taskEntry(void *arg)
    {
        auto *self = static_cast<EspIdfSettingsFetcher *>(arg);
        self->run();

        // marca como finalizado
        xSemaphoreTake(self->_mutex, portMAX_DELAY);
        self->_running = false;
        self->_task = nullptr;
        xSemaphoreGive(self->_mutex);

        vTaskDelete(nullptr);
    }

    void EspIdfSettingsFetcher::run()
    {
        int http_status = -1;

        for (std::uint8_t attempt = 1; attempt <= _req.max_attempts; ++attempt)
        {
            if (_cancel)
            {
                finishAndCallback(StateResult::InvalidState, -1, true);
                return;
            }

            resetBody();
            esp_err_t err = performOnce(http_status);

            if (_cancel)
            {
                finishAndCallback(StateResult::InvalidState, http_status, true);
                return;
            }

            if (err == ESP_OK && http_status >= 200 && http_status < 300)
            {
                finishAndCallback(StateResult::Ok, http_status, false);
                return;
            }

            if (!shouldRetry(err, http_status, attempt))
            {
                // sem retry: retorna erro final
                finishAndCallback(err == ESP_OK ? StateResult::Unknown : map_esp_err(err), http_status, false);
                return;
            }

            const std::uint32_t backoff = computeBackoffMs(attempt);
            vTaskDelay(pdMS_TO_TICKS(backoff));
        }

        finishAndCallback(StateResult::Unknown, http_status, false);
    }

    esp_err_t EspIdfSettingsFetcher::performOnce(int &out_http_status)
    {
        // configura client
        esp_http_client_config_t cfg = {};
        cfg.url = _req.url;
        const bool is_https = (_req.url && (std::strncmp(_req.url, "https://", 8) == 0));
        if (is_https)
        {
            cfg.cert_pem = ISRG_ROOT_X1_PEM;
            cfg.crt_bundle_attach = nullptr;
            cfg.skip_cert_common_name_check = false;
        }
        cfg.method = HTTP_METHOD_GET;
        cfg.timeout_ms = _req.read_timeout_ms;
        cfg.event_handler = &EspIdfSettingsFetcher::httpEventHandler;
        cfg.user_data = this;
        cfg.disable_auto_redirect = false;

        if (_client)
        {
            esp_http_client_cleanup(_client);
            _client = nullptr;
        }

        _client = esp_http_client_init(&cfg);
        if (!_client)
            return ESP_ERR_NO_MEM;

        // timeout de conexão separado
        esp_http_client_set_timeout_ms(_client, _req.read_timeout_ms);
        // Alguns IDFs distinguem connect/read internamente; mantemos simples.

        // headers
        for (std::size_t i = 0; i < _req.headers_count; ++i)
        {
            const auto &h = _req.headers[i];
            if (h.key && h.value)
            {
                esp_http_client_set_header(_client, h.key, h.value);
            }
        }

        // perform (bloqueia só a task do fetcher)
        esp_err_t err = esp_http_client_perform(_client);
        if (err != ESP_OK)
        {
            out_http_status = esp_http_client_get_status_code(_client);
            return err;
        }

        out_http_status = esp_http_client_get_status_code(_client);
        return ESP_OK;
    }

    esp_err_t EspIdfSettingsFetcher::httpEventHandler(esp_http_client_event_t *evt)
    {
        auto *self = static_cast<EspIdfSettingsFetcher *>(evt->user_data);
        return self ? self->onHttpEvent(evt) : ESP_FAIL;
    }

    esp_err_t EspIdfSettingsFetcher::onHttpEvent(esp_http_client_event_t *evt)
    {
        if (_cancel)
            return ESP_FAIL;

        switch (evt->event_id)
        {
        case HTTP_EVENT_ON_DATA:
            if (evt->data && evt->data_len > 0)
            {
                if (!appendBody((const char *)evt->data, evt->data_len))
                {
                    // body excedeu limite -> aborta
                    return ESP_ERR_NO_MEM;
                }
            }
            break;

        default:
            break;
        }
        return ESP_OK;
    }

    void EspIdfSettingsFetcher::resetBody()
    {
        _body_len = 0;
        if (_body)
            _body[0] = '\0';
    }

    bool EspIdfSettingsFetcher::appendBody(const char *data, int len)
    {
        if (!_body || !data || len <= 0)
            return true;

        const std::size_t need = _body_len + (std::size_t)len;
        if (need > _body_cap)
        {
            return false;
        }

        std::memcpy(_body + _body_len, data, (std::size_t)len);
        _body_len = need;
        _body[_body_len] = '\0';
        return true;
    }

    bool EspIdfSettingsFetcher::shouldRetry(esp_err_t err, int http_status, std::uint8_t attempt) const
    {
        if (attempt >= _req.max_attempts)
            return false;

        // cancelamento sempre interrompe
        if (_cancel)
            return false;

        // Transporte/timeout/DNS etc -> retry
        if (err != ESP_OK)
            return true;

        // HTTP codes
        if (http_status >= 500 && http_status <= 599)
            return true; // servidor
        if (http_status == 408 || http_status == 429)
            return true; // timeout / rate limit
        if (http_status >= 400 && http_status <= 499)
            return _req.retry_on_http_4xx;

        return false;
    }

    std::uint32_t EspIdfSettingsFetcher::computeBackoffMs(std::uint8_t attempt) const
    {
        // exponential backoff: base * 2^(attempt-1), capped, com jitter 0..base
        std::uint32_t exp = _req.backoff_base_ms << (attempt - 1);
        exp = std::min(exp, _req.backoff_max_ms);

        const std::uint32_t jitter = esp_random() % (_req.backoff_base_ms + 1);
        const std::uint32_t total = std::min(exp + jitter, _req.backoff_max_ms);

        return std::max<std::uint32_t>(total, 50);
    }

    void EspIdfSettingsFetcher::finishAndCallback(iotsmartsys::core::common::StateResult err, int http_status, bool cancelled)
    {
        SettingsFetchResult r;
        r.err = err;
        r.http_status = http_status;
        r.cancelled = cancelled;
        r.body = _body;
        r.body_len = _body_len;

        // callback SEMPRE, para o chamador decidir fallback
        if (_cb)
        {
            _cb(r, _user_ctx);
        }
    }
} // namespace iotsmartsys::platform::espressif