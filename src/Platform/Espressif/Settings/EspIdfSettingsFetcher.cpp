// Only compile this implementation on ESP32 targets
#include "Config/BuildConfig.h"
#if defined(ESP32) && IOTSMARTSYS_SETTINGS_FETCH_ENABLED

#include "Platform/Espressif/Settings/EspIdfSettingsFetcher.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

#include <cstring>
#include <algorithm>
#include <ctime>
#include <string>
#include "Infra/Certs/GoogleTrustServicesGTSRootR4.h"
#include "Infra/Certs/LetsEncryptISRGRootX1.h"

extern "C"
{
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_crt_bundle.h"
}

namespace iotsmartsys::platform::espressif
{
    using namespace iotsmartsys::core::settings;
    using iotsmartsys::core::common::StateResult;
    static const char *TAG = "SettingsFetcher";
    static constexpr std::time_t kMinValidEpoch = 1704067200; // 2024-01-01 00:00:00 UTC

    static bool isSystemTimeValid()
    {
        const std::time_t now = std::time(nullptr);
        return now >= kMinValidEpoch;
    }

    static bool isNetworkReady()
    {
        return iotsmartsys::core::ConnectivityGate::instance().isNetworkReady();
    }

    static std::string extractHostFromUrl(const char *url)
    {
        if (!url || url[0] == '\0')
        {
            return {};
        }

        const char *schemeEnd = std::strstr(url, "://");
        const char *hostStart = schemeEnd ? (schemeEnd + 3) : url;
        if (*hostStart == '\0' || *hostStart == ':')
        {
            return {};
        }

        const char *hostEnd = hostStart;
        while (*hostEnd && *hostEnd != ':' && *hostEnd != '/' && *hostEnd != '?')
        {
            ++hostEnd;
        }

        return std::string(hostStart, static_cast<std::size_t>(hostEnd - hostStart));
    }

    static const char *nullableString(const char *value)
    {
        return value ? value : "(null)";
    }

    static bool startsWith(const std::string &value, const char *prefix)
    {
        return prefix && value.rfind(prefix, 0) == 0;
    }

    static std::string buildHttpFallbackUrl(const char *url)
    {
        if (!url)
        {
            return {};
        }

        std::string fallback(url);
        if (!startsWith(fallback, "https://"))
        {
            return {};
        }

        if (extractHostFromUrl(url) != "api.iotsmartsys.tech")
        {
            return {};
        }

        fallback.replace(0, std::strlen("https://"), "http://");
        return fallback;
    }

    static bool hostMatches(const std::string &host, const char *suffix)
    {
        if (!suffix)
        {
            return false;
        }

        const std::size_t suffixLen = std::strlen(suffix);
        if (host.size() < suffixLen)
        {
            return false;
        }

        return host.compare(host.size() - suffixLen, suffixLen, suffix) == 0;
    }

    static const char *selectServerCaForUrl(const char *url)
    {
        const std::string host = extractHostFromUrl(url);
        if (host.empty())
        {
            return ISRG_ROOT_X1_PEM;
        }

        if (host == "api.iotsmartsys.tech" || hostMatches(host, ".iotsmartsys.tech"))
        {
            return GTS_ROOT_R4_PEM;
        }

        return ISRG_ROOT_X1_PEM;
    }

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

    EspIdfSettingsFetcher::EspIdfSettingsFetcher(iotsmartsys::core::ILogger &logger)
        : _logger(logger)
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
        iotsmartsys::core::ConnectivityGate::instance().setBits(iotsmartsys::core::ConnectivityGate::APP_NETWORK_BUSY);

        int http_status = -1;
        esp_err_t lastErr = ESP_FAIL;
        const std::string primaryUrl = _req.url ? _req.url : "";
        std::string fallbackUrl;

        for (std::uint8_t phase = 0; phase < 2; ++phase)
        {
            const bool useFallback = (phase == 1);
            if (useFallback)
            {
                if (fallbackUrl.empty() || !shouldUseHttpFallback(lastErr, http_status, primaryUrl.c_str()))
                {
                    break;
                }
                if (!isNetworkReady())
                {
                    _logger.warn("SettingsFetcher", "HTTP_FALLBACK skipped: network not ready. ConnectivityGate.bits=0x%08lx.",
                                 (unsigned long)iotsmartsys::core::ConnectivityGate::instance().bits());
                    finishAndCallback(StateResult::NetworkDown, -1, false);
                    return;
                }
                _logger.warn("SettingsFetcher", "HTTPS_PRIMARY failed by transport err=%s(%d) status=%d. Trying HTTP_FALLBACK.",
                             esp_err_to_name(lastErr),
                             static_cast<int>(lastErr),
                             http_status);
            }

            const std::string &activeUrl = useFallback ? fallbackUrl : primaryUrl;
            const char *protocolLabel = useFallback ? "HTTP_FALLBACK" : (startsWith(activeUrl, "https://") ? "HTTPS_PRIMARY" : "HTTP_PRIMARY");

            for (std::uint8_t attempt = 1; attempt <= _req.max_attempts; ++attempt)
            {
                if (!isNetworkReady())
                {
                    _logger.warn("SettingsFetcher", "API request aborted before protocol=%s attempt=%u: network not ready. ConnectivityGate.bits=0x%08lx.",
                                 protocolLabel,
                                 (unsigned)attempt,
                                 (unsigned long)iotsmartsys::core::ConnectivityGate::instance().bits());
                    finishAndCallback(StateResult::NetworkDown, -1, false);
                    return;
                }

                if (_cancel)
                {
                    finishAndCallback(StateResult::InvalidState, -1, true);
                    return;
                }

                resetBody();
                lastErr = performOnce(activeUrl.c_str(), protocolLabel, http_status);

                if (_cancel)
                {
                    finishAndCallback(StateResult::InvalidState, http_status, true);
                    return;
                }

                if (lastErr == ESP_OK && http_status >= 200 && http_status < 300)
                {
                    _logger.info("SettingsFetcher", "API settings request succeeded using protocol=%s http=%d.",
                                 protocolLabel,
                                 http_status);
                    finishAndCallback(StateResult::Ok, http_status, false);
                    return;
                }

                if (!shouldRetry(lastErr, http_status, attempt))
                {
                    break;
                }

                const std::uint32_t backoff = computeBackoffMs(attempt);
                vTaskDelay(pdMS_TO_TICKS(backoff));
            }

            if (!useFallback)
            {
                fallbackUrl = buildHttpFallbackUrl(primaryUrl.c_str());
            }
        }

        finishAndCallback(lastErr == ESP_OK ? StateResult::Unknown : map_esp_err(lastErr), http_status, false);
    }

    esp_err_t EspIdfSettingsFetcher::performOnce(const char *url, const char *protocol_label, int &out_http_status)
    {
        esp_http_client_config_t cfg = {};
        cfg.url = url;

        const bool is_https = (url && (std::strncmp(url, "https://", 8) == 0));
        if (is_https)
        {
            const std::uint32_t waitBudgetMs = std::max<std::uint32_t>(_req.connect_timeout_ms, 3000);
            std::uint32_t waitedMs = 0;
            while (!_cancel && !isSystemTimeValid() && waitedMs < waitBudgetMs)
            {
                vTaskDelay(pdMS_TO_TICKS(200));
                waitedMs += 200;
            }
            if (!isSystemTimeValid())
            {
                _logger.warn("SettingsFetcher", "System time is not synchronized yet; postponing HTTPS fetch.");
                return ESP_ERR_TIMEOUT;
            }

            cfg.cert_pem = selectServerCaForUrl(url);
            cfg.crt_bundle_attach = nullptr;
            const std::string host = extractHostFromUrl(url);
            _logger.info("SettingsFetcher", "HTTPS trust source: pinned CA for host=%s (%s).",
                         host.c_str(),
                         (cfg.cert_pem == GTS_ROOT_R4_PEM) ? "GTS_ROOT_R4" : "ISRG_ROOT_X1");
            cfg.skip_cert_common_name_check = false;
        }
        cfg.method = HTTP_METHOD_GET;
        cfg.timeout_ms = _req.read_timeout_ms;
        cfg.event_handler = &EspIdfSettingsFetcher::httpEventHandler;
        cfg.user_data = this;
        cfg.disable_auto_redirect = false;

        _logger.info("SettingsFetcher", "API request protocol=%s url=%s",
                     protocol_label ? protocol_label : "(null)",
                     nullableString(cfg.url));
        _logger.info("SettingsFetcher", "HTTP_CONFIG_FULL_URL url=%s", nullableString(cfg.url));
        _logger.info("SettingsFetcher", "HTTP_CONFIG config.url=%s config.host=%s config.path=%s config.port=%d config.transport_type=%d",
                     nullableString(cfg.url),
                     nullableString(cfg.host),
                     nullableString(cfg.path),
                     static_cast<int>(cfg.port),
                     static_cast<int>(cfg.transport_type));

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
        const esp_err_t err = esp_http_client_perform(_client);
        out_http_status = esp_http_client_get_status_code(_client);
        esp_http_client_cleanup(_client);
        _client = nullptr;

        if (err != ESP_OK)
        {
            _logger.error("SettingsFetcher", "HTTP perform failed: %s (%d), status=%d, url=%s",
                          esp_err_to_name(err),
                          (int)err,
                          out_http_status,
                          url ? url : "");
            return err;
        }

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

    bool EspIdfSettingsFetcher::shouldUseHttpFallback(esp_err_t err, int http_status, const char *url) const
    {
        if (_cancel || err == ESP_OK || http_status > 0)
        {
            return false;
        }

        if (!url || std::strncmp(url, "https://", 8) != 0)
        {
            return false;
        }

        if (err == ESP_ERR_NO_MEM || err == ESP_ERR_INVALID_ARG || err == ESP_ERR_INVALID_STATE)
        {
            return false;
        }

        return true;
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
        iotsmartsys::core::ConnectivityGate::instance().clearBits(iotsmartsys::core::ConnectivityGate::APP_NETWORK_BUSY);

        SettingsFetchResult r;
        r.err = err;
        r.http_status = http_status;
        r.cancelled = cancelled;
        r.body = _body;
        r.body_len = _body_len;

        // write raw response text complete to log
       // _logger.info("EspIdfSettingsFetcher", "HTTP Response: %s", r.body);

        // callback SEMPRE, para o chamador decidir fallback
        if (_cb)
        {
            _cb(r, _user_ctx);
        }
    }
} // namespace iotsmartsys::platform::espressif

#endif // defined(ESP32) && IOTSMARTSYS_SETTINGS_FETCH_ENABLED
