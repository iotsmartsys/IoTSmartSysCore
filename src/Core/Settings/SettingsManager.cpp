#include "Contracts/Settings/SettingsManager.h"
#include "Contracts/Common/StateResult.h"

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
        _mutex = xSemaphoreCreateMutex();
    }

    SettingsManager::~SettingsManager()
    {
        if (_mutex)
        {
            vSemaphoreDelete((SemaphoreHandle_t)_mutex);
            _mutex = nullptr;
        }
    }

    iotsmartsys::core::common::StateResult SettingsManager::init()
    {
        using namespace iotsmartsys::core::common;

        if (!_mutex)
            return StateResult::NoMem;

        Settings loaded;
        // provider returns platform result; o provider agora retorna Error
        const StateResult perr = _provider.load(loaded);
        const StateResult err = perr;

        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
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
        xSemaphoreGive((SemaphoreHandle_t)_mutex);

        // Gate: Available somente quando cache (NVS) carregar OK.
        if (err == StateResult::Ok)
        {
            iotsmartsys::core::settings::ISettingsGate &gate = _settingsGate;
            xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
            xSemaphoreGive((SemaphoreHandle_t)_mutex);

            gate.signalAvailable();
        }

        return err;
    }

    iotsmartsys::core::common::StateResult SettingsManager::refreshFromApiAsync(const SettingsFetchRequest &req)
    {
        using namespace iotsmartsys::core::common;

        if (!_mutex)
            return StateResult::NoMem;

        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        _state = SettingsManagerState::FetchingFromApi;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);

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
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const bool v = _has_current;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        return v;
    }

    SettingsManagerState SettingsManager::state() const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const auto s = _state;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        return s;
    }

    bool SettingsManager::copyCurrent(Settings &out) const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        if (!_has_current)
        {
            xSemaphoreGive((SemaphoreHandle_t)_mutex);
            return false;
        }
        out = _current;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        return true;
    }

    void SettingsManager::setUpdatedCallback(SettingsUpdatedCallback cb, void *user_ctx)
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        _updated_cb = cb;
        _updated_ctx = user_ctx;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
    }

    SettingsManagerStats SettingsManager::stats() const
    {
        xSemaphoreTake((SemaphoreHandle_t)_mutex, portMAX_DELAY);
        const auto s = _stats;
        xSemaphoreGive((SemaphoreHandle_t)_mutex);
        return s;
    }

    void SettingsManager::onFetchCompletedStatic(const SettingsFetchResult &res, void *ctx)
    {
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

    void SettingsManager::setState(SettingsManagerState s)
    {
        _state = s;
    }

    void SettingsManager::onFetchCompleted(const SettingsFetchResult &res)
    {
        // Nunca trava: callback vem da task do fetcher, é rápido.
        if (res.cancelled)
        {
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
            gate.signalError(gateErr);

            return;
        }

        // Parse
        Settings parsed;
        const iotsmartsys::core::common::StateResult perr = _parser.parse(res.body, parsed);
        if (perr != iotsmartsys::core::common::StateResult::Ok)
        {
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
    }
} // namespace iotsmartsys::core::settings