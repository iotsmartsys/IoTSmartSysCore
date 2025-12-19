#pragma once
#include <cstdint>
#include <cstddef>

#include "Contracts/Connectivity/ConnectivityGate.h"
#include "Contracts/Settings/SettingsGate.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Transports/IMqttClient.h"

namespace iotsmartsys::app
{

    struct RetryPolicy
    {
        uint32_t initialBackoffMs{1000};
        uint32_t maxBackoffMs{60000};
        uint32_t jitterMs{250};
        uint8_t maxFastRetries{5};
    };

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    class MqttService
    {
    public:
        explicit MqttService(iotsmartsys::core::IMqttClient &client,
                             iotsmartsys::core::ILogger &log)
            : _client(client), _logger(log), _time(nullptr) {}

        void begin(const iotsmartsys::core::MqttConfig &cfg,
                   iotsmartsys::core::settings::ISettingsGate &settingsGate,
                   const RetryPolicy &policy = RetryPolicy{})
        {
            _logger.info("MQTT", "MqttService::begin()");

            _time = &iotsmartsys::core::Time::get();
            if (!_time)
            {
                _logger.warn("MQTT", "Time provider is not set yet");
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
                gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
            }
            _settingsGate = &settingsGate;
            _settingsReady = false;
            _lastSettingsReady = false;
            _lastStatusLogAtMs = 0;
            const auto gateSubErr = _settingsGate->runWhenReady(
                iotsmartsys::core::settings::SettingsReadyLevel::Available,
                &MqttService::onSettingsReadyThunk,
                this);
            if (gateSubErr != iotsmartsys::core::common::Error::Ok)
            {
                _logger.warn("MQTT", "SettingsGate subscription failed (err=%d). MQTT will stay blocked by SettingsReady.", (int)gateSubErr);
            }

            _subCount = 0;
            _qHead = _qTail = _qCount = 0;

            _logger.info("MQTT", "Initializing MQTT client...");
            _client.setOnMessage(&MqttService::onMessageThunk, this);
            _logger.info("MQTT", "MQTT client initialized.");
            _client.begin(_cfg);

            _logger.info("MQTT", "Scheduling initial connection...");
            // No connect immediately; await handle() calls
            _state = State::BackoffWaiting;
            const uint32_t now = _time ? _time->nowMs() : 0;
            _nextActionAtMs = now; // connect on first handle()
        }

        void handle()
        {
            const uint32_t now = _time ? _time->nowMs() : 0;

            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            const bool networkReady = gate.isNetworkReady();
            const bool settingsReady = _settingsReady;
            const bool canConnect = networkReady && settingsReady;

            // Snapshot periódico para diagnosticar facilmente "por que não conectou"
            if (_statusLogEveryMs && (_lastStatusLogAtMs == 0 || (now - _lastStatusLogAtMs) >= _statusLogEveryMs))
            {
                const int32_t until = (_nextActionAtMs > now) ? (int32_t)(_nextActionAtMs - now) : 0;
                _logger.info(
                    "MQTT",
                    "Status: state=%s net=%d settings=%d client=%d attempt=%lu nextInMs=%ld",
                    stateToStr(_state),
                    (int)networkReady,
                    (int)settingsReady,
                    (int)_client.isConnected(),
                    (unsigned long)_attempt,
                    (long)until);
                _lastStatusLogAtMs = now;
            }

            // Se os dois gates ficaram prontos enquanto está Idle, agenda conexão.
            if (canConnect && _state == State::Idle)
            {
                _logger.info("MQTT", "Gates ready and state=Idle; scheduling connection now.");
                _state = State::BackoffWaiting;
                _nextActionAtMs = now;
            }

            // Logs claros de transição de rede (evento latched)
            if (networkReady != _lastNetworkReady)
            {
                if (networkReady)
                {
                    _logger.info("MQTT", "NetworkReady=TRUE (Wi-Fi+IP). MQTT pode conectar.");
                }
                else
                {
                    _logger.warn("MQTT", "NetworkReady=FALSE (Wi-Fi/IP caiu). Pausando MQTT.");
                }
                _lastNetworkReady = networkReady;
            }

            // Logs claros de transição de settings (evento latched)
            if (settingsReady != _lastSettingsReady)
            {
                if (settingsReady)
                {
                    _logger.info("MQTT", "SettingsReady=TRUE (cache OK). MQTT pode conectar quando NetworkReady=TRUE.");
                }
                else
                {
                    _logger.warn("MQTT", "SettingsReady=FALSE (cache ainda não carregou). Bloqueando MQTT.");
                }
                _lastSettingsReady = settingsReady;
            }

            // Gate composto: MQTT só pode operar quando rede (Wi-Fi+IP) E settings (cache ok) estiverem prontos.
            if (!canConnect)
            {
                if (!networkReady)
                    _logger.warn("MQTT", "Blocking MQTT: NetworkReady=FALSE (Wi-Fi/IP caiu).");
                else
                    _logger.warn("MQTT", "Blocking MQTT: SettingsReady=FALSE (cache ainda não carregou).");

                if (_state == State::Connecting || _state == State::Online)
                {
                    _logger.warn("MQTT", "Stopping MQTT due to canConnect=FALSE");
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
                    _logger.warn("MQTT", "Connect timeout (soft-timeout). Scheduling retry.");
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

        // QoS0: se offline -> enfileira (se tiver espaço)
        bool publish(const char *topic, const void *payload, std::size_t len, bool retain = false)
        {
            if (_client.isConnected())
            {
                return _client.publish(topic, payload, len, retain);
            }
            return enqueue(topic, payload, len, retain);
        }

        bool subscribe(const char *topic)
        {
            // guarda para resubscribe
            if (_subCount < MaxTopics)
            {
                _subs[_subCount++] = topic;
            }
            else
            {
                _logger.warn("MQTT", "Subscribe list full; topic=%s", topic);
                return false;
            }

            if (_client.isConnected())
            {
                return _client.subscribe(topic);
            }
            return true;
        }

        // callback opcional para entregar mensagens à sua camada de roteamento
        void setOnMessage(iotsmartsys::core::MqttOnMessageFn cb, void *user)
        {
            _userMsgCb = cb;
            _userMsgUser = user;
        }

        bool isOnline() const { return _state == State::Online && _client.isConnected(); }

    private:
        iotsmartsys::core::settings::ISettingsGate *_settingsGate{nullptr};
        bool _settingsReady{false};
        bool _lastSettingsReady{false};

        enum class State : uint8_t
        {
            Idle,
            Connecting,
            Online,
            BackoffWaiting
        };

        struct QueuedMsg
        {
            const char *topic; // ponteiro para string estática/literal (sem copiar)
            bool retain;
            uint16_t len;
            uint8_t payload[MaxPayload];
        };

    private:
        static const char *stateToStr(State s)
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

        void startConnect()
        {
            // Gate redundante (segurança): nunca tenta MQTT antes de settings estar pronto
            if (!_settingsReady)
            {
                _logger.warn("MQTT", "startConnect blocked: SettingsReady=FALSE (cache ainda não carregou). state=%s", stateToStr(_state));
                _state = State::Idle;
                _nextActionAtMs = 0;
                return;
            }

            // Gate redundante (segurança): nunca tenta MQTT sem Wi-Fi + IP
            {
                auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                if (!gate.isNetworkReady())
                {
                    _logger.warn("MQTT", "startConnect blocked: NetworkReady=FALSE (Wi-Fi/IP não pronto). state=%s", stateToStr(_state));
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

        void scheduleRetry()
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

            _logger.warn("MQTT", "Retry in %lu ms (attempt=%lu)",
                         (unsigned long)backoff, (unsigned long)_attempt);
        }

        uint32_t computeBackoffMs() const
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

        void resubscribeAll()
        {
            for (std::size_t i = 0; i < _subCount; ++i)
            {
                _client.subscribe(_subs[i]);
            }
        }

        void drainQueue()
        {
            // drena 1 por chamada para não travar o loop
            if (_qCount == 0 || !_client.isConnected())
                return;

            QueuedMsg &m = _queue[_qHead];
            _client.publish(m.topic, m.payload, m.len, m.retain);

            _qHead = (_qHead + 1) % QueueLen;
            _qCount--;
        }

        bool enqueue(const char *topic, const void *payload, std::size_t len, bool retain)
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

        static void onMessageThunk(void *user, const iotsmartsys::core::MqttMessageView &msg)
        {
            auto *self = static_cast<MqttService *>(user);
            if (!self)
                return;
            if (self->_userMsgCb)
                self->_userMsgCb(self->_userMsgUser, msg);
        }

        static void onSettingsReadyThunk(iotsmartsys::core::settings::SettingsReadyLevel level, void *ctx)
        {
            auto *self = static_cast<MqttService *>(ctx);
            if (!self)
                return;
            self->onSettingsReady(level);
        }

        void onSettingsReady(iotsmartsys::core::settings::SettingsReadyLevel level)
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

    private:
        iotsmartsys::core::IMqttClient &_client;
        iotsmartsys::core::ILogger &_logger;
        iotsmartsys::core::ITimeProvider *_time;

        iotsmartsys::core::MqttConfig _cfg{};
        RetryPolicy _policy{};

        State _state{State::Idle};
        uint32_t _attempt{0};
        uint32_t _nextActionAtMs{0};
        bool _lastNetworkReady{false};
        uint32_t _lastStatusLogAtMs{0};
        uint32_t _statusLogEveryMs{5000};

        // subscribe list (sem heap): guarde só tópicos "const char*"
        const char *_subs[MaxTopics]{};
        std::size_t _subCount{0};

        // queue ring-buffer (sem heap)
        QueuedMsg _queue[QueueLen]{};
        std::size_t _qHead{0}, _qTail{0}, _qCount{0};

        // user callback
        iotsmartsys::core::MqttOnMessageFn _userMsgCb{nullptr};
        void *_userMsgUser{nullptr};
    };

} // namespace iotsmartsys::app