#ifdef ESP8266
#include "Platform/Esp8266/Settings/Esp8266SettingsFetcher.h"
#include "Contracts/Common/StateResult.h"

#include <cstring>
#include <algorithm>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#else
#include <HTTPClient.h>
#endif

namespace iotsmartsys::platform::esp8266
{
    using namespace iotsmartsys::core::settings;
    using iotsmartsys::core::common::StateResult;

    Esp8266SettingsFetcher::Esp8266SettingsFetcher(iotsmartsys::core::ILogger &logger)
        : _logger(logger)
    {
    }

    Esp8266SettingsFetcher::~Esp8266SettingsFetcher()
    {
        if (_body)
        {
            free(_body);
            _body = nullptr;
        }
    }

    bool Esp8266SettingsFetcher::isRunning() const
    {
        return _running;
    }

    void Esp8266SettingsFetcher::cancel()
    {
        _cancel = true;
    }

    iotsmartsys::core::common::StateResult Esp8266SettingsFetcher::start(const SettingsFetchRequest &req,
                                                                         SettingsFetchCallback cb,
                                                                         void *user_ctx)
    {
        if (!cb || !req.url || req.url[0] == '\0')
            return StateResult::InvalidArg;

        if (_running)
            return StateResult::InvalidState;

        _req = req;
        _cb = cb;
        _user_ctx = user_ctx;
        _cancel = false;

        if (_body)
        {
            free(_body);
            _body = nullptr;
        }

        _body_cap = std::max<std::size_t>(req.max_body_bytes, 256);
        _body = (char *)malloc(_body_cap + 1);
        if (!_body)
            return StateResult::NoMem;
        resetBody();

        _running = true;

        // synchronous implementation: perform the fetch now, call callback, return
        int http_status = -1;
        StateResult final_err = StateResult::Unknown;

        for (std::uint8_t attempt = 1; attempt <= _req.max_attempts; ++attempt)
        {
            if (_cancel)
            {
                final_err = StateResult::InvalidState;
                break;
            }

            resetBody();

            // Use HTTPClient (works for HTTP and HTTPS on typical Arduino cores)
            HTTPClient http;

            const bool is_https = (_req.url && (std::strncmp(_req.url, "https://", 8) == 0));
            int code = -1;

            _logger.debug("Esp8266SettingsFetcher", "HTTP request attempt=%u url=%s https=%s",
                          (unsigned)attempt, _req.url ? _req.url : "(null)", is_https ? "true" : "false");

            if (is_https)
            {
                BearSSL::WiFiClientSecure client;
                // Accept server certificate (insecure) for now; embedding root certs is more involved
                client.setInsecure();
                // Reduce TLS buffer sizes to fit ESP8266 RAM constraints
                client.setBufferSizes(512, 512);

                if (!http.begin(client, _req.url))
                {
                    _logger.warn("Esp8266SettingsFetcher", "http.begin failed for https url=%s", _req.url ? _req.url : "(null)");
                    final_err = StateResult::InvalidArg;
                    http.end();
                    break;
                }

                // headers
                for (std::size_t i = 0; i < _req.headers_count; ++i)
                {
                    const auto &h = _req.headers[i];
                    if (h.key && h.value)
                    {
                        http.addHeader(h.key, h.value);
                    }
                }

                http.setTimeout((int)_req.read_timeout_ms);

                code = http.GET();

                if (code <= 0)
                {
                    _logger.warn("Esp8266SettingsFetcher", "HTTP GET failed (https). code=%d err=%s",
                                 code, http.errorToString(code).c_str());
                    char ssl_err[96];
                    ssl_err[0] = '\0';
                    client.getLastSSLError(ssl_err, sizeof(ssl_err));
                    if (ssl_err[0] != '\0')
                    {
                        _logger.warn("Esp8266SettingsFetcher", "TLS last error: %s", ssl_err);
                    }
                }
            }
            else
            {
                WiFiClient client;
                if (!http.begin(client, _req.url))
                {
                    _logger.warn("Esp8266SettingsFetcher", "http.begin failed for http url=%s", _req.url ? _req.url : "(null)");
                    final_err = StateResult::InvalidArg;
                    http.end();
                    break;
                }

                // headers
                for (std::size_t i = 0; i < _req.headers_count; ++i)
                {
                    const auto &h = _req.headers[i];
                    if (h.key && h.value)
                    {
                        http.addHeader(h.key, h.value);
                    }
                }

                http.setTimeout((int)_req.read_timeout_ms);

                code = http.GET();
                if (code <= 0)
                {
                    _logger.warn("Esp8266SettingsFetcher", "HTTP GET failed (http). code=%d err=%s",
                                 code, http.errorToString(code).c_str());
                }
            }

            if (code > 0)
            {
                http_status = code;

                // read stream in chunks
                WiFiClient *stream = http.getStreamPtr();
                if (stream)
                {
                    uint8_t buf[512];
                    while (stream->available() && !_cancel)
                    {
                        int read = stream->readBytes((char *)buf, sizeof(buf));
                        if (read <= 0)
                            break;
                        if (!appendBody((const char *)buf, (std::size_t)read))
                        {
                            // exceeded buffer
                            break;
                        }
                    }
                }
                else
                {
                    // fallback: getString (may allocate)
                    String s = http.getString();
                    if (s.length() > 0)
                        appendBody(s.c_str(), (std::size_t)s.length());
                }

                http.end();

                if (_cancel)
                {
                    final_err = StateResult::InvalidState;
                    break;
                }

                if (http_status >= 200 && http_status < 300)
                {
                    final_err = StateResult::Ok;
                    break;
                }
                else
                {
                    // decide retry
                    bool retry = false;
                    if (http_status >= 500 && http_status <= 599)
                        retry = true;
                    if (http_status == 408 || http_status == 429)
                        retry = true;
                    if (http_status >= 400 && http_status <= 499)
                        retry = _req.retry_on_http_4xx;

                    if (!retry)
                    {
                        final_err = StateResult::Unknown;
                        break;
                    }
                }
            }
            else
            {
                // transport error
                final_err = StateResult::IoError;
                http.end();
                // retry
            }

            // backoff
            if (attempt < _req.max_attempts && !_cancel)
            {
                const std::uint32_t backoff = std::min(_req.backoff_base_ms << (attempt - 1), _req.backoff_max_ms);
                delay((unsigned long)backoff);
            }
        }

        finishAndCallback(final_err, http_status, _cancel);
        _running = false;

        return final_err;
    }

    void Esp8266SettingsFetcher::resetBody()
    {
        _body_len = 0;
        if (_body)
            _body[0] = '\0';
    }

    bool Esp8266SettingsFetcher::appendBody(const char *data, std::size_t len)
    {
        if (!_body || !data || len == 0)
            return true;

        const std::size_t need = _body_len + len;
        if (need > _body_cap)
            return false;

        std::memcpy(_body + _body_len, data, len);
        _body_len = need;
        _body[_body_len] = '\0';
        return true;
    }

    void Esp8266SettingsFetcher::finishAndCallback(iotsmartsys::core::common::StateResult err, int http_status, bool cancelled)
    {
        SettingsFetchResult r;
        r.err = err;
        r.http_status = http_status;
        r.cancelled = cancelled;
        r.body = _body;
        r.body_len = _body_len;

        // log
        _logger.info("Esp8266SettingsFetcher", "HTTP Response: %s", r.body ? r.body : "(null)");

        if (_cb)
            _cb(r, _user_ctx);
    }
} // namespace iotsmartsys::platform::esp8266
#endif // ESP8266
