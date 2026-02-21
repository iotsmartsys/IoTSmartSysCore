#include "Contracts/Settings/SettingsManager.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

using namespace iotsmartsys::core::common;

namespace iotsmartsys::core::settings
{
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
        }
        else
        {

            _stats.cache_load_fail++;
            _stats.last_err = perr;
            // Não muda _current se não carregou; pode continuar vazio
            _state = SettingsManagerState::Idle;
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
            // _logger->error("SettingsManager", "Settings fetch cancelled (branch)");
            _stats.last_err = iotsmartsys::core::common::StateResult::InvalidState;
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Idle);
            _settingsGate.setLevel(SettingsReadyLevel::None, iotsmartsys::core::common::StateResult::InvalidState);
            return;
        }

        // Falha de HTTP/transporte ou status != 2xx => mantém redundância (NVS/memória)
        if (pending.fetch_err != iotsmartsys::core::common::StateResult::Ok || pending.http_status < 200 || pending.http_status >= 300)
        {
            // _logger->error("SettingsManager", "fetch failed: err=%d http=%d", (int)pending.fetch_err, pending.http_status);
            updateStatsFail(pending.fetch_err, pending.http_status);
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Error);

            // Se transporte foi Ok mas status HTTP falhou, use InvalidState como fallback.
            const auto gateErr = (pending.fetch_err != iotsmartsys::core::common::StateResult::Ok)
                                     ? pending.fetch_err
                                     : iotsmartsys::core::common::StateResult::InvalidState;
            // _logger->error("SettingsManager", "signaling gate error %d", (int)gateErr);
            _settingsGate.setLevel(SettingsReadyLevel::None, gateErr);
            return;
        }

        if (!pending.has_parsed)
        {
            // _logger->error("SettingsManager", "parse failed: %d", (int)pending.parse_err);
            _stats.parse_fail++;
            _stats.last_err = pending.parse_err;
            _stats.last_http_status = pending.http_status;
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Error);
            _settingsGate.setLevel(SettingsReadyLevel::None, pending.parse_err);
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
            // _logger->info("SettingsManager", "fetched settings differ from current; updating in-memory and saving to NVS");

            _current = candidate;
            _has_current = true;

            const iotsmartsys::core::common::StateResult serr = _provider.save(_current);
            if (serr != iotsmartsys::core::common::StateResult::Ok)
            {
                // _logger->error("SettingsManager", "nvs save failed: %d", (int)serr);
                _stats.nvs_save_fail++;
                _stats.last_err = serr;
                _stats.last_http_status = pending.http_status;
                _stats.api_fetch_ok++;
                setState(SettingsManagerState::Ready);
            }
            else
            {
                // _logger->info("SettingsManager", "nvs save OK");
                _stats.api_fetch_ok++;
                _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
                _stats.last_http_status = pending.http_status;
                setState(SettingsManagerState::Ready);
            }
        }
        else
        {
            // _logger->info("SettingsManager", "fetched settings identical to current; skipping NVS save");
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

        if (_updated_cb)
        {
            const Settings snapshot = _current;
            _updated_cb(snapshot, _updated_ctx);
        }
        // _logger->info("SettingsManager", "onFetchCompleted() finished, callback invoked=%s", _updated_cb ? "true" : "false");
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
        if (_settingsGate.level() == SettingsReadyLevel::Available && networkReady)
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
            return;
        }
        _syncUrlBuffer = _current.api.url;
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
            _logger->info("[SettingsManager] API refresh started.");
        }
        else
        {
            _logger->warn("[SettingsManager] Failed to start API refresh.");
        }
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
