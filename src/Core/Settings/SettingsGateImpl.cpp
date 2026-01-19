#include "Core/Settings/SettingsGateImpl.h"

namespace iotsmartsys::core::settings
{

    using iotsmartsys::core::common::StateResult;

    bool SettingsGateImpl::isAtLeast(SettingsReadyLevel have, SettingsReadyLevel want)
    {
        return static_cast<uint8_t>(have) >= static_cast<uint8_t>(want);
    }

    SettingsReadyLevel SettingsGateImpl::level() const
    {
        return _level;
    }

    void SettingsGateImpl::signalAvailable()
    {
        // regra sua: Available só quando cache OK
        setLevel(SettingsReadyLevel::Available, StateResult::Ok);
    }

    void SettingsGateImpl::signalSynced()
    {
        // Synced pode acontecer mesmo sem cache (API OK)
        setLevel(SettingsReadyLevel::Synced, StateResult::Ok);
    }

    void SettingsGateImpl::signalSyncing()
    {
        setLevel(SettingsReadyLevel::Syncing, StateResult::Ok);
    }

    void SettingsGateImpl::signalError(StateResult err)
    {
        _last_err = err;
        // Não rebaixa o nível — gate é “latch”: depois que ficou pronto, não volta.
    }

    void SettingsGateImpl::setLevel(SettingsReadyLevel newLevel, StateResult lastErr)
    {
        SettingsGateCallback toCall[kMaxSubs];
        void *toCtx[kMaxSubs];
        int n = 0;

        _last_err = lastErr;

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

        // executa fora da iteração (2 fases)
        for (int i = 0; i < n; i++)
        {
            if (toCall[i])
                toCall[i](_level, toCtx[i]);
        }
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
        for (auto &s : _subs)
        {
            if (!s.used)
            {
                s.used = true;
                s.want = want;
                s.cb = cb;
                s.ctx = user_ctx;
                return StateResult::Ok;
            }
        }
        return StateResult::NoMem; // sem slot
    }

} // namespace
