#include "Core/Services/MqttService.h"
#include "Contracts/Providers/ServiceProvider.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>

#ifdef ESP32
#include <esp_system.h>
#endif

namespace iotsmartsys::app
{

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    MqttService<MaxTopics, QueueLen, MaxPayload>::MqttService(iotsmartsys::core::IMqttClient &client,
                                                              iotsmartsys::core::ILogger &log,
                                                              iotsmartsys::core::settings::ISettingsGate &settingsGate,
                                                              iotsmartsys::core::settings::IReadOnlySettingsProvider &settingsProvider)
        : _client(client), _logger(log), _time(nullptr), _settingsProvider(settingsProvider), _settingsGate(settingsGate)
    {
        // Ensure MQTT_CONNECTED bit is clear at start
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::begin(const iotsmartsys::core::TransportConfig &cfg)
    {
        return begin(cfg, RetryPolicy{});
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::begin(const iotsmartsys::core::TransportConfig &cfg,
                                                             const RetryPolicy &policy)
    {
        _logger.info("MQTT", "MqttService::begin()");

        _time = &iotsmartsys::core::Time::get();
        _logger.info("MQTT", "Time provider set.");
        if (!_time)
        {
            _logger.debug("MQTT", "Time provider is not set yet");
        }
        _cfg = cfg;
        _policy = policy;
        _attempt = 0;
        _nextActionAtMs = 0;
        _state = State::Idle;
        _lastNetworkReady = false;
        _lastSettingsReady = false;

        // Ensure MQTT_CONNECTED bit is clear at start
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            _logger.debug("MQTT", "Clearing MQTT_CONNECTED bit");
            gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
            _logger.debug("MQTT", "MQTT_CONNECTED bit cleared");
        }
        _settingsReady = false;
        _lastSettingsReady = false;
        _lastStatusLogAtMs = 0;
        _logger.debug("MQTT", "Subscribing to SettingsGate for SettingsReady events...");

        const auto gateSubErr = _settingsGate.runWhenReady(
            iotsmartsys::core::settings::SettingsReadyLevel::Available,
            &MqttService::onSettingsReadyThunk,
            this);
        if (gateSubErr != iotsmartsys::core::common::StateResult::Ok)
        {
            _logger.warn("MQTT", "SettingsGate subscription failed (err=%d). MQTT will stay blocked by SettingsReady.", (int)gateSubErr);
        }

        // _subCount = 0;
        _qHead = _qTail = _qCount = 0;
        subscribe(cfg.subscribeTopic);

        const char *uriSafe = cfg.uri ? cfg.uri : "(null)";
        _logger.info("MQTT", "Initializing MQTT client uri='%s'", uriSafe);
        _client.setOnMessage(&MqttService::onMessageThunk, this);
        _client.setOnConnected(&MqttService::onConnectedThunk, this);
        _client.setOnDisconnected(&MqttService::onDisconnectedThunk, this);
        _logger.info("MQTT", "MQTT client initialized.");
        const bool clientInitOk = _client.begin(_cfg);

        _logger.info("MQTT", "Scheduling initial connection...");
        // No connect immediately; await handle() calls
        _state = State::BackoffWaiting;
        const uint32_t now = _time ? _time->nowMs() : 0;
        _nextActionAtMs = now; // connect on first handle()
        return clientInitOk;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::handle()
    {
        const uint32_t now = _time ? _time->nowMs() : 0;

        auto &gate = iotsmartsys::core::ConnectivityGate::instance();
        const bool networkReady = gate.isNetworkReady();
        const bool canConnect = networkReady && _settingsReady;

        // Snapshot periódico para diagnosticar facilmente "por que não conectou"
        if (_statusLogEveryMs && (_lastStatusLogAtMs == 0 || (now - _lastStatusLogAtMs) >= _statusLogEveryMs))
        {
            const int32_t until = (_nextActionAtMs > now) ? (int32_t)(_nextActionAtMs - now) : 0;
            _logger.debug(
                "MQTT",
                "Status: state=%s net=%d settings=%d client=%d attempt=%lu nextInMs=%ld",
                stateToStr(_state),
                (int)networkReady,
                (int)_settingsReady,
                (int)_client.isConnected(),
                (unsigned long)_attempt,
                (long)until);
            _lastStatusLogAtMs = now;
        }

        // Se os dois gates ficaram prontos enquanto está Idle, agenda conexão.
        if (canConnect && _state == State::Idle)
        {
            _logger.debug("MQTT", "Gates ready and state=Idle; scheduling connection now.");
            _state = State::BackoffWaiting;
            _nextActionAtMs = now;
        }

        // Logs claros de transição de rede (evento latched)
        if (networkReady != _lastNetworkReady)
        {
            if (networkReady)
            {
                _logger.debug("MQTT", "NetworkReady=TRUE (Wi-Fi+IP). MQTT pode conectar.");
            }
            else
            {
                _logger.debug("MQTT", "NetworkReady=FALSE (Wi-Fi/IP). Pausando MQTT.");
            }
            _lastNetworkReady = networkReady;
        }

        auto *settingsGate = &_settingsGate;
        _settingsReady = settingsGate->level() >= iotsmartsys::core::settings::SettingsReadyLevel::Available;

        // Logs claros de transição de settings (evento latched)
        if (_settingsReady != _lastSettingsReady)
        {
            if (_settingsReady)
            {
                _logger.debug("MQTT", "SettingsReady=TRUE (cache OK). MQTT pode conectar quando NetworkReady=TRUE.");
            }
            else
            {
                _logger.debug("MQTT", "SettingsReady=FALSE (cache ainda não carregou). Bloqueando MQTT.");
            }
            _lastSettingsReady = _settingsReady;
        }

        // Gate composto: MQTT só pode operar quando rede (Wi-Fi+IP) E settings (cache ok) estiverem prontos.
        if (!canConnect)
        {
            if (!networkReady)
                _logger.debug("MQTT", "Blocking MQTT: NetworkReady=FALSE (Wi-Fi/IP).");
            else
                _logger.debug("MQTT", "Blocking MQTT: SettingsReady=FALSE (cache ainda não carregou).");

            if (_state == State::Connecting || _state == State::Online)
            {
                _logger.debug("MQTT", "Stopping MQTT due to canConnect=FALSE");
                _client.stop();
            }

            // garante bit limpo mesmo se já estiver em backoff/idle
            gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);

            // Fica em Idle aguardando os gates liberarem.
            _state = State::Idle;
            _nextActionAtMs = 0;
            return;
        }

        switch (_state)
        {
        case State::Idle:
            break;

        case State::Connecting:
            if (_client.isConnected())
            {
                _state = State::Online;
                _attempt = 0;
                _logger.info("MQTT", "Online");
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.setBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
                }
                resubscribeAll();
                drainQueue();
            }
            else if (_nextActionAtMs && now >= _nextActionAtMs)
            {
                _logger.debug("MQTT", "Connect timeout (soft-timeout). Scheduling retry.");
                scheduleRetry();
            }
            break;

        case State::BackoffWaiting:
            if (now >= _nextActionAtMs)
                startConnect();
            break;

        case State::Online:
            if (!_client.isConnected())
            {
                _logger.warn("MQTT", "Disconnected");
                {
                    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                    gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
                }
                scheduleRetry();
            }
            else
            {
                // tenta drenar aos poucos (sem travar)
                drainQueue();
            }
            break;
        }
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::publish(const char *topic, const void *payload, std::size_t len, bool retain)
    {
        if (_client.isConnected())
        {
            return _client.publish(topic, payload, len, retain);
        }
        return enqueue(topic, payload, len, retain);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::republish(const iotsmartsys::core::TransportMessageView &msg)
    {
        if (_client.isConnected())
        {
            return _client.republish(msg);
        }
        return enqueue(msg.topic, msg.payload, msg.payloadLen, msg.retain);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::subscribe(const char *topic)
    {
        if (!topic || std::strlen(topic) == 0)
        {
            _logger.debug("MQTT", "Empty topic in subscribe()");
            return false;
        }
        
        // guarda para resubscribe
        // deve evitar que tópicos duplicados sejam adicionados
        for (std::size_t i = 0; i < _subCount; ++i)
        {
            if (_subs[i] == topic)
            {
                _logger.debug("MQTT", "Topic already in subscribe list; topic=%s", topic);
                return true;
            }
        }

        if (_subCount < MaxTopics)
        {
            _logger.info("MQTT", "(func subscribe) Subscribing to topic: %s", topic);
            _subs[_subCount++] = (topic ? topic : "");
        }
        else
        {
            _logger.debug("MQTT", "Subscribe list full; topic=%s", topic);
            return false;
        }

        if (_client.isConnected())
        {
            _logger.info("MQTT", "(func subscribe) Client connected, subscribing to topic: %s", topic);
            return _client.subscribe(topic);
        }
        else
        {
            _logger.info("MQTT", "(func subscribe) Client not connected, enqueuing topic: %s", topic);
        }
        return true;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::setOnMessage(iotsmartsys::core::TransportOnMessageFn cb, void *user)
    {
        _userMsgCb = cb;
        _userMsgUser = user;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::setOnConnected(iotsmartsys::core::TransportOnConnectedFn cb, void *user)
    {
        _userConnectedCb = cb;
        _userConnectedUser = user;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn cb, void *user)
    {
        _userDisconnectedCb = cb;
        _userDisconnectedUser = user;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::isOnline() const
    {
        return _state == State::Online && _client.isConnected();
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::isConnected() const
    {
        return _client.isConnected();
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    const char *MqttService<MaxTopics, QueueLen, MaxPayload>::stateToStr(State s)
    {
        switch (s)
        {
        case State::Idle:
            return "Idle";
        case State::Connecting:
            return "Connecting";
        case State::Online:
            return "Online";
        case State::BackoffWaiting:
            return "BackoffWaiting";
        default:
            return "?";
        }
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::startConnect()
    {
        // Gate redundante (segurança): nunca tenta MQTT antes de settings estar pronto
        if (!_settingsReady)
        {
            _logger.debug("MQTT", "startConnect blocked: SettingsReady=FALSE (cache ainda não carregou). state=%s", stateToStr(_state));
            _state = State::Idle;
            _nextActionAtMs = 0;
            return;
        }

        iotsmartsys::core::settings::Settings settings;
        if (!_settingsProvider.copyCurrent(settings))
        {
            _logger.debug("MQTT", "startConnect aborted: settings not available yet.");
            _state = State::Idle;
            _nextActionAtMs = 0;
            return;
        }

        // Build persistent strings inside the service so we don't point to
        // temporaries owned by the local 'settings' object (that would dangle
        // after this function returns). Use a full URI (protocol://host:port)
        // if possible.
        const std::string proto = settings.mqtt.primary.protocol.empty() ? std::string("mqtt") : settings.mqtt.primary.protocol;
        const std::string host = settings.mqtt.primary.host.empty() ? std::string() : settings.mqtt.primary.host;
        const int port = (settings.mqtt.primary.port > 0) ? settings.mqtt.primary.port : 1883;
        if (!host.empty())
        {
            _uriStr = proto + "://" + host + ":" + std::to_string(port);
        }
        else
        {
            _uriStr.clear();
        }

        if (_uriStr.empty())
        {
            _logger.debug("MQTT", "startConnect aborted: MQTT host/uri not configured.");
            _state = State::Idle;
            _nextActionAtMs = 0;
            return;
        }

        _usernameStr = settings.mqtt.primary.user;
        _passwordStr = settings.mqtt.primary.password;
        _clientIdStr = settings.clientId ? settings.clientId : std::string();

        // Point _cfg members to the service-owned strings (stable storage)
        _cfg.uri = _uriStr.empty() ? nullptr : _uriStr.c_str();
        _cfg.username = _usernameStr.empty() ? nullptr : _usernameStr.c_str();
        _cfg.password = _passwordStr.empty() ? nullptr : _passwordStr.c_str();
        _cfg.clientId = _clientIdStr.empty() ? nullptr : _clientIdStr.c_str();
        _cfg.keepAliveSec = settings.mqtt.primary.keepAliveSec;
        _cfg.cleanSession = settings.mqtt.primary.cleanSession;

        begin(_cfg, _policy);

        // Gate redundante (segurança): nunca tenta MQTT sem Wi-Fi + IP
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            if (!gate.isNetworkReady())
            {
                _logger.debug("MQTT", "startConnect blocked: NetworkReady=FALSE (Wi-Fi/IP não pronto). state=%s", stateToStr(_state));
                _state = State::BackoffWaiting;
                _nextActionAtMs = _time ? _time->nowMs() + 1000 : 1000;
                return;
            }
        }

        _state = State::Connecting;
        _logger.debug("MQTT", "Starting connection attempt %lu", (unsigned long)(_attempt + 1));
        _nextActionAtMs = (_time ? _time->nowMs() : 0) + 15000; // soft-timeout de conexão
        _logger.info("MQTT", "Connecting (attempt=%lu)", (unsigned long)(_attempt + 1));
        _client.start();
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::scheduleRetry()
    {
        _attempt++;
        const uint32_t now = _time ? _time->nowMs() : 0;
        const uint32_t backoff = computeBackoffMs();

        _client.stop();
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }
        _state = State::BackoffWaiting;
        _nextActionAtMs = now + backoff;

        _logger.debug("MQTT", "Retry in %lu ms (attempt=%lu)",
                      (unsigned long)backoff, (unsigned long)_attempt);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    uint32_t MqttService<MaxTopics, QueueLen, MaxPayload>::computeBackoffMs() const
    {
        uint32_t base = _policy.initialBackoffMs;
        uint8_t exp = (_attempt <= _policy.maxFastRetries) ? _attempt : _policy.maxFastRetries;

        for (uint8_t i = 0; i < exp; ++i)
        {
            if (base > (_policy.maxBackoffMs / 2))
            {
                base = _policy.maxBackoffMs;
                break;
            }
            base *= 2;
        }
        if (base > _policy.maxBackoffMs)
            base = _policy.maxBackoffMs;

        uint32_t jitter = 0;
#ifdef ESP32
        if (_policy.jitterMs)
            jitter = (uint32_t)(esp_random() % (_policy.jitterMs + 1));
#endif
        return base + jitter;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::resubscribeAll()
    {
        _logger.info("MQTT", "Resubscribing to %lu topics", (unsigned long)_subCount);
        for (std::size_t i = 0; i < _subCount; ++i)
        {
            _logger.info("MQTT", "Resubscribing to topic: %s", _subs[i].c_str());
            _client.subscribe(_subs[i].c_str());
        }
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::drainQueue()
    {
        // drena 1 por chamada para não travar o loop
        if (_qCount == 0 || !_client.isConnected())
            return;

        QueuedMsg &m = _queue[_qHead];
        _client.publish(m.topic, m.payload, m.len, m.retain);

        _qHead = (_qHead + 1) % QueueLen;
        _qCount--;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    bool MqttService<MaxTopics, QueueLen, MaxPayload>::enqueue(const char *topic, const void *payload, std::size_t len, bool retain)
    {
        if (len > MaxPayload)
        {
            _logger.warn("MQTT", "Payload too big (%lu > %lu). Drop.",
                         (unsigned long)len, (unsigned long)MaxPayload);
            return false;
        }
        if (_qCount >= QueueLen)
        {
            _logger.warn("MQTT", "Queue full. Drop.");
            return false;
        }

        QueuedMsg &m = _queue[_qTail];
        m.topic = topic;
        m.retain = retain;
        m.len = (uint16_t)len;
        for (std::size_t i = 0; i < len; ++i)
            m.payload[i] = ((const uint8_t *)payload)[i];

        _qTail = (_qTail + 1) % QueueLen;
        _qCount++;
        return true;
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::onMessageThunk(void *user, const iotsmartsys::core::TransportMessageView &msg)
    {
        auto *self = static_cast<MqttService *>(user);
        if (!self)
            return;
        if (self->_userMsgCb)
            self->_userMsgCb(self->_userMsgUser, msg);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::onConnectedThunk(void *user, const iotsmartsys::core::TransportConnectedView &info)
    {
        auto *self = static_cast<MqttService *>(user);
        if (!self)
            return;
        if (self->_userConnectedCb)
            self->_userConnectedCb(self->_userConnectedUser, info);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::onDisconnectedThunk(void *user)
    {
        auto *self = static_cast<MqttService *>(user);
        if (!self)
            return;
        if (self->_userDisconnectedCb)
            self->_userDisconnectedCb(self->_userDisconnectedUser);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::onSettingsReadyThunk(iotsmartsys::core::settings::SettingsReadyLevel level, void *ctx)
    {
        auto *self = static_cast<MqttService *>(ctx);
        if (!self)
            return;
        self->onSettingsReady(level);
    }

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    void MqttService<MaxTopics, QueueLen, MaxPayload>::onSettingsReady(iotsmartsys::core::settings::SettingsReadyLevel level)
    {
        (void)level;
        _settingsReady = true;

        // se rede já está pronta, agenda conectar no próximo handle()
        if (_time)
        {
            const uint32_t now = _time->nowMs();
            if (_state == State::Idle || (_state == State::BackoffWaiting && _nextActionAtMs == 0))
            {
                _state = State::BackoffWaiting;
                _nextActionAtMs = now;
            }
        }

        _logger.info("MQTT", "SettingsReady=TRUE (cache OK). MQTT liberado quando NetworkReady=TRUE.");
    }

    // Explicit instantiation for the common configuration used in this project
    template class MqttService<12, 16, 256>;

} // namespace iotsmartsys::app
