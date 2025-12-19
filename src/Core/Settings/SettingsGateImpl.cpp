#include "Core/Settings/SettingsGateImpl.h"

namespace iotsmartsys::core::settings
{

    using iotsmartsys::core::common::StateResult;

    SettingsGateImpl::SettingsGateImpl()
    {
        _mutex = xSemaphoreCreateMutex();
    }

    SettingsGateImpl::~SettingsGateImpl()
    {
        if (_mutex)
        {
            vSemaphoreDelete(_mutex);
            _mutex = nullptr;
        }
    }

    void SettingsGateImpl::lock() const
    {
        if (_mutex)
            xSemaphoreTake(_mutex, portMAX_DELAY);
    }

    void SettingsGateImpl::unlock() const
    {
        if (_mutex)
            xSemaphoreGive(_mutex);
    }

    bool SettingsGateImpl::isAtLeast(SettingsReadyLevel have, SettingsReadyLevel want)
    {
        return static_cast<uint8_t>(have) >= static_cast<uint8_t>(want);
    }

    SettingsReadyLevel SettingsGateImpl::level() const
    {
        lock();
        const auto l = _level;
        unlock();
        return l;
    }

    void SettingsGateImpl::signalAvailable()
    {
        // regra sua: Available só quando cache OK
        setLevel(SettingsReadyLevel::Available);
    }

    void SettingsGateImpl::signalSynced()
    {
        // Synced pode acontecer mesmo sem cache (API OK)
        setLevel(SettingsReadyLevel::Synced);
    }

    void SettingsGateImpl::signalError(StateResult err)
    {
        lock();
        _last_err = err;
        unlock();
        // Não rebaixa o nível — gate é “latch”: depois que ficou pronto, não volta.
    }

    StateResult SettingsGateImpl::runWhenReady(SettingsReadyLevel want, SettingsGateCallback cb, void *user_ctx)
    {
        if (!cb)
            return StateResult::InvalidArg;

        // Se já está pronto, executa imediatamente (fora do lock)
        const auto cur = level();
        if (isAtLeast(cur, want))
        {
            cb(cur, user_ctx);
            return StateResult::Ok;
        }

        // Senão, registra para depois
        lock();
        for (auto &s : _subs)
        {
            if (!s.used)
            {
                s.used = true;
                s.want = want;
                s.cb = cb;
                s.ctx = user_ctx;
                unlock();
                return StateResult::Ok;
            }
        }
        unlock();
        return StateResult::NoMem; // sem slot
    }

    void SettingsGateImpl::setLevel(SettingsReadyLevel newLevel)
    {
        SettingsGateCallback toCall[kMaxSubs];
        void *toCtx[kMaxSubs];
        int n = 0;

        lock();

        // só sobe o nível
        if (static_cast<uint8_t>(newLevel) > static_cast<uint8_t>(_level))
        {
            _level = newLevel;
        }

        // coleta callbacks que já podem rodar
        for (auto &s : _subs)
        {
            if (s.used && isAtLeast(_level, s.want))
            {
                toCall[n] = s.cb;
                toCtx[n] = s.ctx;
                n++;
                s.used = false;
            }
        }

        unlock();

        // executa fora do lock (evita deadlock)
        for (int i = 0; i < n; i++)
        {
            if (toCall[i])
                toCall[i](_level, toCtx[i]);
        }
    }

} // namespace