#include "Contracts/Connections/WiFiManager.h"

namespace iotsmartsys::app
{

    WiFiManager::WiFiManager(iotsmartsys::core::ILogger &log)
        : _log(log), _timeProvider(nullptr)
    {
    }

    void WiFiManager::begin(const WiFiConfig &cfg)
    {
        // initialize time provider here (setup() should have called Time::setProvider)
        _timeProvider = &iotsmartsys::core::Time::get();

        _cfg = cfg;
        _attempt = 0;
        _gotIp = false;
        _state = State::Idle;
        _nextActionAtMs = 0;

#ifdef ESP32
        // event-driven (mais robusto)
        _eventId = WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t)
                                { this->onWiFiEvent(event); });
#endif

        WiFi.mode(WIFI_STA);
        WiFi.persistent(_cfg.persistent);

        // Eu prefiro controlar reconexão manualmente.
        // Se você quiser testar com autoReconnect=true, dá, mas pode ficar “duas lógicas brigando”.
#ifdef ESP32
        WiFi.setAutoReconnect(_cfg.autoReconnect);
#endif

        startConnect();
    }

    bool WiFiManager::isConnected() const
    {
#ifdef ESP32
        return _gotIp && WiFi.status() == WL_CONNECTED;
#else
        return WiFi.status() == WL_CONNECTED;
#endif
    }

    const char *WiFiManager::stateName() const
    {
        switch (_state)
        {
        case State::Idle:
            return "Idle";
        case State::Connecting:
            return "Connecting";
        case State::Connected:
            return "Connected";
        case State::BackoffWaiting:
            return "BackoffWaiting";
        default:
            return "?";
        }
    }

    void WiFiManager::handle()
    {
    const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());

        switch (_state)
        {
        case State::Idle:
            // nada
            break;

        case State::Connecting:
            // fallback para casos em que evento não venha / travou
            // (sem bloquear)
            if (isConnected())
            {
                _state = State::Connected;
                _connectedAtMs = now;
                _attempt = 0;
                _log.info("WIFI", "Connected. IP=%s", WiFi.localIP().toString().c_str());
            }
            else if (_nextActionAtMs != 0 && now >= _nextActionAtMs)
            {
                scheduleRetry();
            }
            break;

        case State::BackoffWaiting:
            if (now >= _nextActionAtMs)
                startConnect();
            break;

        case State::Connected:
            if (!isConnected())
            {
                // evita ficar reconectando freneticamente se a rede está instável
                if (now - _connectedAtMs < _cfg.reconnectMinUptimeMs)
                {
                    _log.warn("WIFI", "Flapping detected; delaying reconnect");
                }
                scheduleRetry();
            }
            break;
        }
    }

    void WiFiManager::startConnect()
    {
        if (!_cfg.ssid || !_cfg.ssid[0])
        {
            _log.error("WIFI", "SSID not set");
            return;
        }

        _gotIp = false;
        _state = State::Connecting;

        // timeout “soft”: se passar, entra em retry (sem bloquear)
    const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        _nextActionAtMs = now + 15000; // 15s

        _log.info("WIFI", "Connecting to SSID=%s (attempt=%lu)", _cfg.ssid, (unsigned long)(_attempt + 1));

        WiFi.disconnect(true); // limpa estado anterior
        WiFi.begin(_cfg.ssid, _cfg.password);
    }

    void WiFiManager::scheduleRetry()
    {
        _attempt++;

    const uint32_t now = (_timeProvider ? _timeProvider->nowMs() : millis());
        const uint32_t backoff = computeBackoffMs();

        _nextActionAtMs = now + backoff;
        _state = State::BackoffWaiting;

        _log.warn("WIFI", "Retry in %lu ms (attempt=%lu)", (unsigned long)backoff, (unsigned long)_attempt);
    }

    uint32_t WiFiManager::computeBackoffMs() const
    {
        // backoff exponencial com teto + jitter
        uint32_t base = _cfg.initialBackoffMs;

        // até maxFastRetries cresce rápido; depois estabiliza no maxBackoffMs
        uint8_t exp = (_attempt <= _cfg.maxFastRetries) ? _attempt : _cfg.maxFastRetries;
        for (uint8_t i = 0; i < exp; ++i)
        {
            if (base > (_cfg.maxBackoffMs / 2))
            {
                base = _cfg.maxBackoffMs;
                break;
            }
            base *= 2;
        }
        if (base > _cfg.maxBackoffMs)
            base = _cfg.maxBackoffMs;

        uint32_t jitter = 0;
        if (_cfg.jitterMs)
        {
            // jitter simples (não críptico): ok pra backoff
            jitter = (uint32_t)(esp_random() % (_cfg.jitterMs + 1));
        }

        return base + jitter;
    }

#ifdef ESP32
    void WiFiManager::onWiFiEvent(WiFiEvent_t event)
    {
        switch (event)
        {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            _gotIp = true;
            // o handle() consolida estado e loga
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            _gotIp = false;
            // não reconecta aqui direto (pra não reconectar em ISR/event flood)
            // o handle() vai perceber e aplicar retry/backoff
            break;

        default:
            break;
        }
    }
#endif

} // namespace iotsmartsys::app