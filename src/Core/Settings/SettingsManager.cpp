#include "Contracts/Settings/SettingsManager.h"
#include "Contracts/Common/StateResult.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
}
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
        _mutex = xSemaphoreCreateMutex();
    }

    SettingsManager::~SettingsManager()
    {
        _logger->debug("SettingsManager", "destructed");
        if (_mutex)
        {
            vSemaphoreDelete((SemaphoreHandle_t)_mutex);
            _mutex = nullptr;
        }
    }

    iotsmartsys::core::common::StateResult SettingsManager::init()
    {
        using namespace iotsmartsys::core::common;

        _logger->debug("SettingsManager", "init() called");

        if (!_mutex)
            return StateResult::NoMem;

        Settings loaded;
        // provider returns platform result; o provider agora retorna Error
        const StateResult perr = _provider.load(loaded);
        const StateResult err = perr;

        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        if (err == StateResult::Ok)
        {
            _logger->debug("SettingsManager", "cache loaded OK");
            _current = loaded;
            _has_current = true;
            _stats.cache_load_ok++;
            _state = SettingsManagerState::LoadedFromCache;
        }
        else
        {
            _logger->debug("SettingsManager", "cache load failed (%d)", (int)perr);
            _stats.cache_load_fail++;
            _stats.last_err = perr;
            // Não muda _current se não carregou; pode continuar vazio
            _state = SettingsManagerState::Idle;
        }
        xSemaphoreGive((SemaphoreHandle_t)_mutex);

        // Gate: Available somente quando cache (NVS) carregar OK.
        if (err == StateResult::Ok)
        {
            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            gate.signalAvailable();
        }

        _logger->debug("SettingsManager", "init() returning %d", (int)err);
        return err;
    }

    iotsmartsys::core::common::StateResult SettingsManager::refreshFromApiAsync(const SettingsFetchRequest &req)
    {
        using namespace iotsmartsys::core::common;

        if (!_mutex)
            return StateResult::NoMem;

        _logger->debug("SettingsManager", "refreshFromApiAsync() called: url=%s", req.url);
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        _state = SettingsManagerState::FetchingFromApi;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);

        // fetcher já roda em task própria; aqui retorna rápido
        const StateResult r = _fetcher.start(req, &SettingsManager::onFetchCompletedStatic, this);
        _logger->debug("SettingsManager", "fetcher.start returned %d", (int)r);
        return r;
    }

    void SettingsManager::cancel()
    {
        _logger->debug("SettingsManager", "cancel() requested");
        _fetcher.cancel();
    }

    bool SettingsManager::hasCurrent() const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const bool v = _has_current;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        _logger->debug("SettingsManager", "hasCurrent() => %s", v ? "true" : "false");
        return v;
    }

    SettingsManagerState SettingsManager::state() const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const auto s = _state;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        _logger->debug("SettingsManager", "state() => %d", (int)s);
        return s;
    }

    bool SettingsManager::copyCurrent(Settings &out) const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        if (!_has_current)
        {
            xSemaphoreGive((SemaphoreHandle_t)_mutex);
            _logger->debug("SettingsManager", "copyCurrent() => no current settings");
            return false;
        }
        out = _current;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        _logger->debug("SettingsManager", "copyCurrent() => success");
        return true;
    }

    void SettingsManager::setUpdatedCallback(SettingsUpdatedCallback cb, void *user_ctx)
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        _updated_cb = cb;
        _updated_ctx = user_ctx;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        _logger->debug("SettingsManager", "setUpdatedCallback() set");
    }

    SettingsManagerStats SettingsManager::stats() const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const auto s = _stats;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        _logger->debug("SettingsManager", "stats() requested");
        return s;
    }

    void SettingsManager::onFetchCompletedStatic(const SettingsFetchResult &res, void *ctx)
    {
        // Não podemos usar _logger aqui (método estático). Use o logger do ServiceProvider.
        if (iotsmartsys::core::ServiceProvider::instance().logger())
        {
            auto &logger = *iotsmartsys::core::ServiceProvider::instance().logger();
            logger.debug("SettingsManager", "onFetchCompletedStatic invoked");
        }
        auto *self = static_cast<SettingsManager *>(ctx);
        if (self)
            self->onFetchCompleted(res);
    }

    void SettingsManager::updateStatsFail(iotsmartsys::core::common::StateResult err, int http_status)
    {
        _logger->debug("SettingsManager", "updateStatsFail err=%d http=%d", (int)err, http_status);
        _stats.api_fetch_fail++;
        _stats.last_err = err;
        _stats.last_http_status = http_status;
    }

    void SettingsManager::setState(SettingsManagerState s)
    {
        _logger->debug("SettingsManager", "setState(%d)", (int)s);
        _state = s;
    }

    void SettingsManager::onFetchCompleted(const SettingsFetchResult &res)
    {
        // Nunca trava: callback vem da task do fetcher, é rápido.
        _logger->debug("SettingsManager", "onFetchCompleted() called (cancelled=%s, err=%d, http=%d)", res.cancelled ? "true" : "false", (int)res.err, res.http_status);
        if (res.cancelled)
        {
            _logger->debug("SettingsManager", "Settings fetch cancelled (branch)");
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            _stats.last_err = iotsmartsys::core::common::StateResult::InvalidState;
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Idle);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);
            gate.signalError(iotsmartsys::core::common::StateResult::InvalidState);

            return;
        }

        // Falha de HTTP/transporte ou status != 2xx => mantém redundância (NVS/memória)
        if (res.err != iotsmartsys::core::common::StateResult::Ok || res.http_status < 200 || res.http_status >= 300)
        {
            _logger->debug("SettingsManager", "fetch failed: err=%d http=%d", (int)res.err, res.http_status);
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            updateStatsFail(res.err, res.http_status);
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Error);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            // Se transporte foi Ok mas status HTTP falhou, use InvalidState como fallback.
            const auto gateErr = (res.err != iotsmartsys::core::common::StateResult::Ok)
                                     ? res.err
                                     : iotsmartsys::core::common::StateResult::InvalidState;
            _logger->debug("SettingsManager", "signaling gate error %d", (int)gateErr);
            gate.signalError(gateErr);

            return;
        }

        // Parse
        Settings parsed;
        const iotsmartsys::core::common::StateResult perr = _parser.parse(res.body, parsed);
        if (perr != iotsmartsys::core::common::StateResult::Ok)
        {
            _logger->debug("SettingsManager", "parse failed: %d", (int)perr);
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            _stats.parse_fail++;
            _stats.last_err = perr;
            _stats.last_http_status = res.http_status;
            setState(_has_current ? SettingsManagerState::Ready : SettingsManagerState::Error);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);
            gate.signalError(perr);

            return;
        }

        // Persistir no NVS (redundância). API continua sendo fonte de verdade.
        const iotsmartsys::core::common::StateResult serr = _provider.save(parsed);
        if (serr != iotsmartsys::core::common::StateResult::Ok)
        {
            _logger->debug("SettingsManager", "nvs save failed: %d", (int)serr);
            // Importante: mesmo se falhar NVS, você *pode* usar o settings em memória.
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            _stats.nvs_save_fail++;
            _stats.last_err = serr;
            _stats.last_http_status = res.http_status;

            _current = parsed;
            _has_current = true;
            _stats.api_fetch_ok++;
            setState(SettingsManagerState::Ready);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);
        }
        else
        {
            _logger->debug("SettingsManager", "nvs save OK");
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            _current = parsed;
            _has_current = true;
            _stats.api_fetch_ok++;
            _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
            _stats.last_http_status = res.http_status;
            setState(SettingsManagerState::Ready);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);
        }

        // Gate: Synced quando veio da API (fetch + parse OK) e foi aplicado em memória.
        {
            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            _logger->debug("SettingsManager", "signaling gate Synced");
            gate.signalSynced();
        }

        // Notificar (fora do lock, pra evitar deadlock/callback lento)
        SettingsUpdatedCallback cb = nullptr;
        void *cb_ctx = nullptr;
        Settings snapshot;

        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        cb = _updated_cb;
        cb_ctx = _updated_ctx;
        snapshot = _current;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);

        if (cb)
            cb(snapshot, cb_ctx);
        _logger->debug("SettingsManager", "onFetchCompleted() finished, callback invoked=%s", cb ? "true" : "false");
    }

    void SettingsManager::handle()
    {

        auto &gate = iotsmartsys::core::ConnectivityGate::instance();
        const bool networkReady = gate.isNetworkReady();
        if (_settingsGate.level() == SettingsReadyLevel::Available && networkReady)
        {
            _logger->debug("SettingsManager", "handle(): SettingsGate available, syncFromApi()");
            this->syncFromApi();
        }
        _logger->trace("SettingsManager", "_settingsGate.level() = %d", (int)_settingsGate.level());
    }

    void SettingsManager::saveWiFiOnly(const WifiConfig &wifi)
    {
        _logger->debug("SettingsManager", "saveWiFiOnly() called");
        if (!_mutex)
            return;

        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const auto serr = _provider.saveWiFiOnly(wifi);
        if (serr != iotsmartsys::core::common::StateResult::Ok)
        {
            _logger->debug("SettingsManager", "saveWiFiOnly: NVS save failed: %d", (int)serr);
            _stats.nvs_save_fail++;
            _stats.last_err = serr;
        }
        else
        {
            _logger->debug("SettingsManager", "saveWiFiOnly: NVS save OK");
            _stats.last_err = iotsmartsys::core::common::StateResult::Ok;
        }
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
    }

    void SettingsManager::syncFromApi()
    {
        {
            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            _logger->debug("SettingsManager", "signaling gate Synced");
            gate.signalSyncing();
        }
        _logger->debug("SettingsManager", "syncFromApi() starting");
        // 2) API is source of truth: refresh asynchronously (does not block firmware)
        SettingsFetchRequest req;
        req.url = SETTINGS_API_URL; // define this macro in your env/build_flags

        // Optional headers (example). If you don't need them, leave empty.
        static HttpHeader headers[] = {
            {"x-api-key", API_KEY},
            {"client_id", CLIENT_ID},
            {"Authorization", API_BASIC_AUTH},
        };
        req.headers = headers;
        req.headers_count = sizeof(headers) / sizeof(headers[0]);

        req.connect_timeout_ms = 3000;
        req.read_timeout_ms = 6000;
        req.max_body_bytes = 10 * 1024;
        req.max_attempts = 4;

        const auto startErr = refreshFromApiAsync(req);
        if (startErr == StateResult::Ok)
            _logger->info("[SettingsManager] API refresh started.");
        else
            _logger->warn("[SettingsManager] Failed to start API refresh.");
    }
} // namespace iotsmartsys::core::settings