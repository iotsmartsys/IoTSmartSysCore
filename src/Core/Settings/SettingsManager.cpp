#include "Contracts/Settings/SettingsManager.h"
#include "Config/WifiCredentials.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

#include <cstring>

using namespace iotsmartsys::core::common;

namespace iotsmartsys::core::settings
{
    namespace
    {
        constexpr const char *kDefaultDeviceSettingsEndpoint =
            "devices/:device_id/settings?prefix_auto_format_properties_json=mqtt,firmware,wifi&use_key_value=no";
        constexpr const char *kDefaultSettingsQuery =
            "prefix_auto_format_properties_json=mqtt,firmware,wifi&use_key_value=no";
        constexpr std::uint32_t kApiSyncInitialBackoffMs = 5000;
        constexpr std::uint32_t kApiSyncMaxBackoffMs = 60000;

        bool endsWith(const std::string &value, const std::string &suffix)
        {
            return value.size() >= suffix.size() &&
                   value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        void trimTrailingSlashes(std::string &value)
        {
            while (!value.empty() && value.back() == '/')
            {
                value.pop_back();
            }
        }

        std::string appendDefaultSettingsQueryIfNeeded(std::string url)
        {
            if (url.find('?') == std::string::npos && endsWith(url, "/settings"))
            {
                url += "?";
                url += kDefaultSettingsQuery;
            }
            return url;
        }

        std::string buildSettingsUrl(const Settings &settings)
        {
            if (settings.api.url.empty())
            {
                return {};
            }

            std::string url = settings.api.url;
            if (url.find(":device_id") != std::string::npos || endsWith(url, "/settings"))
            {
                return appendDefaultSettingsQueryIfNeeded(url);
            }

            trimTrailingSlashes(url);

#ifdef IOTSMARTSYS_API_DEVICE_SETTINGS_ENDPOINT
            constexpr const char *endpoint = IOTSMARTSYS_API_DEVICE_SETTINGS_ENDPOINT;
#else
            constexpr const char *endpoint = kDefaultDeviceSettingsEndpoint;
#endif

            if (endpoint == nullptr || *endpoint == '\0')
            {
                return url;
            }

            url += "/";
            url += endpoint;
            return url;
        }

        const char *compiledEnvironmentId()
        {
#ifdef IOTSMARTSYS_ENV_ID
            return IOTSMARTSYS_ENV_ID;
#else
            return "";
#endif
        }

        std::string resolveFirmwareManifest(std::string manifest)
        {
            static constexpr const char *kEnvIdPlaceholder = "{env_id}";
            const char *envId = compiledEnvironmentId();
            if (!envId || envId[0] == '\0')
            {
                return manifest;
            }

            std::size_t pos = 0;
            while ((pos = manifest.find(kEnvIdPlaceholder, pos)) != std::string::npos)
            {
                manifest.replace(pos, std::strlen(kEnvIdPlaceholder), envId);
                pos += std::strlen(envId);
            }

            return manifest;
        }

        void applyCompiledWifiOverride(Settings &settings)
        {
            if (settings.wifi.isValid())
            {
                settings.wifi.syncSelectedLegacyFields();
                return;
            }

            if (iotsmartsys::config::hasWifiCredentials())
            {
                settings.wifi.ssid = iotsmartsys::config::kWifiCredentials[0].ssid;
                settings.wifi.password = iotsmartsys::config::kWifiCredentials[0].password;
                settings.wifi.primary.ssid = settings.wifi.ssid;
                settings.wifi.primary.password = settings.wifi.password;
                settings.wifi.profile = "primary";
                settings.in_config_mode = false;
                return;
            }

#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
            settings.wifi.ssid = WIFI_SSID;
            settings.wifi.password = WIFI_PASSWORD;
            settings.wifi.primary.ssid = settings.wifi.ssid;
            settings.wifi.primary.password = settings.wifi.password;
            settings.wifi.profile = "primary";
            settings.in_config_mode = false;
#endif
        }

        const char *stateResultToStr(StateResult result)
        {
            switch (result)
            {
            case StateResult::Ok:
                return "Ok";
            case StateResult::InvalidArg:
                return "InvalidArg";
            case StateResult::InvalidState:
                return "InvalidState";
            case StateResult::NoMem:
                return "NoMem";
            case StateResult::Timeout:
                return "Timeout";
            case StateResult::Cancelled:
                return "Cancelled";
            case StateResult::NotFound:
                return "NotFound";
            case StateResult::Overflow:
                return "Overflow";
            case StateResult::ParseError:
                return "ParseError";
            case StateResult::IoError:
                return "IoError";
            case StateResult::NotSupported:
                return "NotSupported";
            case StateResult::NetworkDown:
                return "NetworkDown";
            case StateResult::DnsFail:
                return "DnsFail";
            case StateResult::ConnectFail:
                return "ConnectFail";
            case StateResult::TlsFail:
                return "TlsFail";
            case StateResult::HttpError:
                return "HttpError";
            case StateResult::StorageCorrupt:
                return "StorageCorrupt";
            case StateResult::StorageVersionMismatch:
                return "StorageVersionMismatch";
            case StateResult::StorageWriteFail:
                return "StorageWriteFail";
            case StateResult::StorageReadFail:
                return "StorageReadFail";
            case StateResult::Unknown:
            default:
                return "Unknown";
            }
        }
    }

    SettingsManager::SettingsManager(core::providers::ISettingsProvider &provider,
                                     ISettingsFetcher &fetcher,
                                     ISettingsParser &parser,
                                     ISettingsGate &settingsGate)
        : _provider(provider), _fetcher(fetcher), _parser(parser), _settingsGate(settingsGate)
    {
        _logger = iotsmartsys::core::ServiceProvider::instance().logger();
    }

    SettingsManager::~SettingsManager()
    {
    }

    iotsmartsys::core::common::StateResult SettingsManager::init()
    {
        using namespace iotsmartsys::core::common;

        Settings loaded;
        // provider returns platform result; o provider agora retorna Error
        const StateResult perr = _provider.load(loaded);
        const StateResult err = perr;

        if (err == StateResult::Ok)
        {
            _current = loaded;
            _has_current = true;
            _stats.cache_load_ok++;
            _state = SettingsManagerState::LoadedFromCache;
            if (_logger)
            {
                _logger->info("SettingsManager", "Loaded settings from cache. mqttProfile='%s' mqttHost='%s' apiUrl='%s'.",
                              _current.mqtt.profile.c_str(),
                              _current.mqtt.primary.host.c_str(),
                              _current.api.url.c_str());
            }
        }
        else
        {

            _stats.cache_load_fail++;
            _stats.last_err = perr;
            // Não muda _current se não carregou; pode continuar vazio
            _state = SettingsManagerState::Idle;
            if (_logger)
            {
                _logger->warn("SettingsManager", "No cached settings loaded. err=%s(%d).",
                              stateResultToStr(perr),
                              (int)perr);
            }
        }

        // Gate: Available somente quando cache (NVS) carregar OK.
        if (err == StateResult::Ok)
        {
            _settingsGate.setLevel(SettingsReadyLevel::Available, iotsmartsys::core::common::StateResult::Ok);
        }

        return err;
    }

    iotsmartsys::core::common::StateResult SettingsManager::refreshFromApiAsync(const SettingsFetchRequest &req)
    {
        using namespace iotsmartsys::core::common;
        _state = SettingsManagerState::FetchingFromApi;

        // fetcher já roda em task própria; aqui retorna rápido
        const StateResult r = _fetcher.start(req, &SettingsManager::onFetchCompletedStatic, this);
        if (r != StateResult::Ok)
        {
            _state = _has_current ? SettingsManagerState::Ready : SettingsManagerState::Error;
        }
        return r;
    }

    void SettingsManager::cancel()
    {
        _fetcher.cancel();
    }

    bool SettingsManager::hasCurrent() const
    {
        const bool v = _has_current;
        return v;
    }

    SettingsManagerState SettingsManager::state() const
    {
        const auto s = _state;
        return s;
    }

    bool SettingsManager::copyCurrent(Settings &out) const
    {
        if (!_has_current)
        {
            return false;
        }
        out = _current;
        return true;
    }

    void SettingsManager::setUpdatedCallback(SettingsUpdatedCallback cb, void *user_ctx)
    {
        _updated_cb = cb;
        _updated_ctx = user_ctx;
    }

    SettingsManagerStats SettingsManager::stats() const
    {
        const auto s = _stats;
        return s;
    }

    void SettingsManager::onFetchCompletedStatic(const SettingsFetchResult &res, void *ctx)
    {
        // Não podemos usar _logger aqui (método estático). Use o logger do ServiceProvider.
        if (iotsmartsys::core::ServiceProvider::instance().logger())
        {
            auto &logger = *iotsmartsys::core::ServiceProvider::instance().logger();
            logger.info("SettingsManager", "onFetchCompletedStatic invoked");
        }
        auto *self = static_cast<SettingsManager *>(ctx);
        if (self)
            self->onFetchCompleted(res);
    }

    void SettingsManager::updateStatsFail(iotsmartsys::core::common::StateResult err, int http_status)
    {
        _stats.api_fetch_fail++;
        _stats.last_err = err;
        _stats.last_http_status = http_status;
    }

    void SettingsManager::setPendingUpdate(const PendingSettingsUpdate &pending)
    {
        const auto seq = _pendingSeq.load(std::memory_order_relaxed);
        _pendingSeq.store(seq + 1, std::memory_order_release);
        _pendingUpdate = pending;
        _pendingSeq.store(seq + 2, std::memory_order_release);
        _hasPending.store(true, std::memory_order_release);
    }

    void SettingsManager::setState(SettingsManagerState s)
    {
        _state = s;
    }

    void SettingsManager::onFetchCompleted(const SettingsFetchResult &res)
    {
        // Nunca trava: callback vem da task do fetcher, é rápido.
        // _logger->info("SettingsManager", "onFetchCompleted() called (cancelled=%s, err=%d, http=%d)", res.cancelled ? "true" : "false", (int)res.err, res.http_status);
        PendingSettingsUpdate pending{};
        pending.cancelled = res.cancelled;
        pending.fetch_err = res.err;
        pending.http_status = res.http_status;

        if (!res.cancelled && res.err == iotsmartsys::core::common::StateResult::Ok && res.http_status >= 200 && res.http_status < 300)
        {
            Settings parsed;
            const iotsmartsys::core::common::StateResult perr = _parser.parse(res.body, parsed);
            if (perr == iotsmartsys::core::common::StateResult::Ok)
            {
                parsed.firmware.manifest = resolveFirmwareManifest(parsed.firmware.manifest);
                applyCompiledWifiOverride(parsed);
                pending.has_parsed = true;
                pending.parsed = parsed;
            }
            else
            {
                pending.parse_err = perr;
            }
        }

        setPendingUpdate(pending);
    }

    void SettingsManager::applyPendingUpdate(const PendingSettingsUpdate &pending)
    {
        if (pending.cancelled)
        {
            if (_logger)
            {
                _logger->warn("SettingsManager", "API refresh cancelled.");
            }
            _stats.last_err = iotsmartsys::core::common::StateResult::InvalidState;
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Idle);
            _settingsGate.setLevel(_has_current ? SettingsReadyLevel::Available : SettingsReadyLevel::None,
                                   iotsmartsys::core::common::StateResult::InvalidState);
            scheduleApiRetryBackoff();
            return;
        }

        // Falha de HTTP/transporte ou status != 2xx => mantém redundância (NVS/memória)
        if (pending.fetch_err != iotsmartsys::core::common::StateResult::Ok || pending.http_status < 200 || pending.http_status >= 300)
        {
            if (_logger)
            {
                _logger->warn("SettingsManager", "API refresh failed. err=%s(%d) http=%d keepingCurrent=%s.",
                              stateResultToStr(pending.fetch_err),
                              (int)pending.fetch_err,
                              pending.http_status,
                              _has_current ? "true" : "false");
            }
            updateStatsFail(pending.fetch_err, pending.http_status);
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Error);

            // Se transporte foi Ok mas status HTTP falhou, use InvalidState como fallback.
            const auto gateErr = (pending.fetch_err != iotsmartsys::core::common::StateResult::Ok)
                                     ? pending.fetch_err
                                     : iotsmartsys::core::common::StateResult::InvalidState;
            // Keep cached settings usable for MQTT while API sync keeps retrying.
            _settingsGate.setLevel(_has_current ? SettingsReadyLevel::Available : SettingsReadyLevel::None, gateErr);
            scheduleApiRetryBackoff();
            return;
        }

        if (!pending.has_parsed)
        {
            if (_logger)
            {
                _logger->warn("SettingsManager", "API refresh parse failed. err=%s(%d) http=%d keepingCurrent=%s.",
                              stateResultToStr(pending.parse_err),
                              (int)pending.parse_err,
                              pending.http_status,
                              _has_current ? "true" : "false");
            }
            _stats.parse_fail++;
            _stats.last_err = pending.parse_err;
            _stats.last_http_status = pending.http_status;
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Error);
            _settingsGate.setLevel(_has_current ? SettingsReadyLevel::Available : SettingsReadyLevel::None, pending.parse_err);
            scheduleApiRetryBackoff();
            return;
        }

        Settings currentSnapshot = _current;
        const bool hadCurrent = _has_current;
        Settings candidate = currentSnapshot;
        candidate._is_changed = false;
        candidate.applyChanges(pending.parsed);
        const bool changed = candidate.hasChanges();

        if (changed)
        {
            if (_logger)
            {
                _logger->info("SettingsManager", "API settings changed. Applying update and saving cache. mqttProfile='%s' mqttHost='%s' wifiProfile='%s' wifiSsid='%s'.",
                              candidate.mqtt.profile.c_str(),
                              candidate.mqtt.primary.host.c_str(),
                              candidate.wifi.profile.c_str(),
                              candidate.wifi.ssid.c_str());
            }

            _current = candidate;
            _has_current = true;

            const iotsmartsys::core::common::StateResult serr = _provider.save(_current);
            if (serr != iotsmartsys::core::common::StateResult::Ok)
            {
                if (_logger)
                {
                    _logger->warn("SettingsManager", "API settings applied but cache save failed. err=%s(%d) http=%d.",
                                  stateResultToStr(serr),
                                  (int)serr,
                                  pending.http_status);
                }
                _stats.nvs_save_fail++;
                _stats.last_err = serr;
                _stats.last_http_status = pending.http_status;
                _stats.api_fetch_ok++;
                setState(SettingsManagerState::Ready);
            }
            else
            {
                if (_logger)
                {
                    _logger->info("SettingsManager", "API settings applied and cache saved. http=%d callback=%s.",
                                  pending.http_status,
                                  _updated_cb ? "yes" : "no");
                }
                _stats.api_fetch_ok++;
                _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
                _stats.last_http_status = pending.http_status;
                setState(SettingsManagerState::Ready);
            }
        }
        else
        {
            if (_logger)
            {
                _logger->info("SettingsManager", "API settings unchanged. Cache save skipped. http=%d callback=%s.",
                              pending.http_status,
                              _updated_cb ? "yes" : "no");
            }
            if (!hadCurrent)
            {
                _current = candidate;
                _has_current = true;
            }
            _stats.api_fetch_ok++;
            _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
            _stats.last_http_status = pending.http_status;
            setState(SettingsManagerState::Ready);
        }

        // Gate: Synced quando veio da API (fetch + parse OK) e foi aplicado em memória.
        // _logger->info("SettingsManager", "signaling gate Synced");
        _settingsGate.setLevel(SettingsReadyLevel::Synced, iotsmartsys::core::common::StateResult::Ok);
        resetApiRetryBackoff();

        if (_logger)
        {
            _logger->info("SettingsManager", "API refresh completed. changed=%s http=%d mqttProfile='%s' mqttHost='%s' wifiProfile='%s' wifiSsid='%s'.",
                          changed ? "true" : "false",
                          pending.http_status,
                          _current.mqtt.profile.c_str(),
                          _current.mqtt.primary.host.c_str(),
                          _current.wifi.profile.c_str(),
                          _current.wifi.ssid.c_str());
        }

        if (_updated_cb)
        {
            if (_logger)
            {
                _logger->info("SettingsManager", "Invoking settings update callback. changed=%s.", changed ? "true" : "false");
            }
            const Settings snapshot = _current;
            _updated_cb(snapshot, _updated_ctx);
        }
    }

    void SettingsManager::handle()
    {
        if (_hasPending.load(std::memory_order_acquire))
        {
            // _logger->info("SettingsManager", "handle(): applying pending update");
            PendingSettingsUpdate pending{};
            for (;;)
            {
                // _logger->info("SettingsManager", "handle(): checking pending update");
                const auto seq1 = _pendingSeq.load(std::memory_order_acquire);
                if (seq1 & 1U)
                    continue;
                pending = _pendingUpdate;
                const auto seq2 = _pendingSeq.load(std::memory_order_acquire);
                if (seq1 == seq2 && !(seq2 & 1U))
                    break;
            }
            // _logger->info("SettingsManager", "handle(): got pending update, applying");
            _hasPending.store(false, std::memory_order_release);
            // _logger->info("SettingsManager", "handle(): calling applyPendingUpdate()");
            applyPendingUpdate(pending);
        }

        auto &gate = iotsmartsys::core::ConnectivityGate::instance();
        const bool networkReady = gate.isNetworkReady();
        const std::uint64_t nowMs = iotsmartsys::core::Time::get().nowMs();
        if (_settingsGate.level() == SettingsReadyLevel::Available && networkReady && nowMs >= _nextApiSyncAtMs)
        {
            this->syncFromApi();
        }
        //// _logger->info("SettingsManager", "_settingsGate.level() = %d e networkReady = %s (ConnectivityGate.bits=0x%08x)", (int)_settingsGate.level(), networkReady ? " conectado" : "não conectado", gate.bits());
    }

    bool SettingsManager::save(const Settings &settings)
    {

        _current.applyChanges(settings);
        _has_current = true;

        const auto serr = _provider.save(settings);

        if (serr != iotsmartsys::core::common::StateResult::Ok)
        {
            _stats.nvs_save_fail++;
            _stats.last_err = serr;
            return false;
        }
        _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
        return true;
    }

    void SettingsManager::saveWiFiOnly(const WifiConfig &wifi)
    {
        const auto serr = _provider.saveWiFiOnly(wifi);
        if (serr != iotsmartsys::core::common::StateResult::Ok)
        {
            _stats.nvs_save_fail++;
            _stats.last_err = serr;
        }
        else
        {
            _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
        }
    }

    void SettingsManager::syncFromApi()
    {
        if (_fetcher.isRunning())
        {
            _logger->warn("[SettingsManager] syncFromApi() skipped: fetch already running.");
            return;
        }

        {
            _settingsGate.setLevel(SettingsReadyLevel::Syncing, iotsmartsys::core::common::StateResult::Ok);
        }

        SettingsFetchRequest req;

        if (_has_current == false)
        {
            _logger->warn("[SettingsManager] syncFromApi() no current settings, cannot sync.");
            _settingsGate.setLevel(SettingsReadyLevel::None, iotsmartsys::core::common::StateResult::InvalidState);
            scheduleApiRetryBackoff();
            return;
        }
        _syncUrlBuffer = buildSettingsUrl(_current);
        const auto deviceIdPlaceholder = std::string(":device_id");
        const auto placeholderPos = _syncUrlBuffer.find(deviceIdPlaceholder);
        if (placeholderPos != std::string::npos && _current.clientId != nullptr)
        {
            _syncUrlBuffer.replace(placeholderPos, deviceIdPlaceholder.length(), _current.clientId);
        }

        // Keep pointer valid while the async fetcher runs.
        req.url = _syncUrlBuffer.c_str();

        _syncApiKeyBuffer = _current.api.key;
        _syncClientIdBuffer = (_current.clientId != nullptr) ? _current.clientId : "";
        _syncAuthBuffer = _current.api.basic_auth;

        _syncHeaders[0] = {"x-api-key", _syncApiKeyBuffer.c_str()};
        _syncHeaders[1] = {"client_id", _syncClientIdBuffer.c_str()};
        _syncHeaders[2] = {"Authorization", _syncAuthBuffer.c_str()};
        req.headers = _syncHeaders.data();
        req.headers_count = _syncHeaders.size();

        req.connect_timeout_ms = 3000;
        req.read_timeout_ms = 6000;
        req.max_body_bytes = 10 * 1024;
        req.max_attempts = 4;

        const auto startErr = refreshFromApiAsync(req);
        if (startErr == StateResult::Ok)
        {
            _logger->info("SettingsManager", "API refresh started. url='%s' maxAttempts=%u connectTimeout=%lums readTimeout=%lums.",
                          req.url ? req.url : "",
                          (unsigned int)req.max_attempts,
                          (unsigned long)req.connect_timeout_ms,
                          (unsigned long)req.read_timeout_ms);
        }
        else
        {
            _logger->warn("SettingsManager", "Failed to start API refresh. err=%s(%d) url='%s'.",
                          stateResultToStr(startErr),
                          (int)startErr,
                          req.url ? req.url : "");
            _settingsGate.setLevel(_has_current ? SettingsReadyLevel::Available : SettingsReadyLevel::None, startErr);
            scheduleApiRetryBackoff();
        }
    }

    void SettingsManager::resetApiRetryBackoff()
    {
        _apiSyncFailures = 0;
        _nextApiSyncAtMs = 0;
    }

    void SettingsManager::scheduleApiRetryBackoff()
    {
        if (_apiSyncFailures < 15)
        {
            ++_apiSyncFailures;
        }

        std::uint32_t delayMs = kApiSyncInitialBackoffMs;
        for (std::uint8_t i = 1; i < _apiSyncFailures; ++i)
        {
            if (delayMs >= (kApiSyncMaxBackoffMs / 2))
            {
                delayMs = kApiSyncMaxBackoffMs;
                break;
            }
            delayMs *= 2;
        }

        if (delayMs > kApiSyncMaxBackoffMs)
        {
            delayMs = kApiSyncMaxBackoffMs;
        }

        _nextApiSyncAtMs = iotsmartsys::core::Time::get().nowMs() + delayMs;
    }

    void SettingsManager::clear()
    {
        const auto err = _provider.erase();
        if (err != iotsmartsys::core::common::StateResult::Ok)
        {
            // _logger->error("SettingsManager", "eraseCache: NVS erase failed: %d", (int)err);
            _stats.last_err = err;
        }
        else
        {
            // _logger->info("SettingsManager", "eraseCache: NVS erase OK");
            _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
            _current = Settings{};
            _has_current = false;
            _state = SettingsManagerState::Idle;
        }
    }
} // namespace iotsmartsys::core::settings
