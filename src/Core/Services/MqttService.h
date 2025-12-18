#pragma once
#include <cstdint>
#include <cstddef>

#include "Contracts/Connectivity/ConnectivityGate.h"

#include "Contracts/Logging/ILogger.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Transports/IMqttClient.h"

namespace iotsmartsys::app {

struct RetryPolicy {
    uint32_t initialBackoffMs{1000};
    uint32_t maxBackoffMs{60000};
    uint32_t jitterMs{250};
    uint8_t  maxFastRetries{5};
};

template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
class MqttService {
public:
    explicit MqttService(iotsmartsys::core::IMqttClient& client,
                         iotsmartsys::core::ILogger& log)
        : _client(client), _logger(log), _time(nullptr) {}

    void begin(const iotsmartsys::core::MqttConfig& cfg,
               const RetryPolicy& policy = RetryPolicy{})
    {
        _logger.info("MQTT", "MqttService::begin()");

        // resolve time provider at begin() time because the global Time provider
        // might be set after static construction of this service (see main.cpp)
    // Time::get() returns a reference; take its address to obtain pointer safely
    _time = &iotsmartsys::core::Time::get();
        if (!_time) {
            _logger.warn("MQTT", "Time provider is not set yet");
        }
        _cfg = cfg;
        _policy = policy;
        _attempt = 0;
        _nextActionAtMs = 0;
        _state = State::Idle;

        // garante que o estado de MQTT no latch começa limpo
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
        }

        _subCount = 0;
        _qHead = _qTail = _qCount = 0;

        _logger.info("MQTT", "Initializing MQTT client...");
        _client.setOnMessage(&MqttService::onMessageThunk, this);
        _logger.info("MQTT", "MQTT client initialized.");
        _client.begin(_cfg);

        _logger.info("MQTT", "Scheduling initial connection...");
        // não tenta conectar MQTT se ainda não houver Wi-Fi+IP; o handle() vai disparar assim que a rede estiver pronta
        _state = State::BackoffWaiting;
        const uint32_t now = _time ? _time->nowMs() : 0;
        _nextActionAtMs = now; // tenta já na próxima chamada de handle()
    }

    void handle()
    {
        const uint32_t now = _time ? _time->nowMs() : 0;

        switch (_state) {
            case State::Idle:
                break;

            case State::Connecting:
                if (_client.isConnected()) {
                    _state = State::Online;
                    _attempt = 0;
                    _logger.info("MQTT", "Online");
                    {
                        auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                        gate.setBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
                    }
                    resubscribeAll();
                    drainQueue();
                } else if (_nextActionAtMs && now >= _nextActionAtMs) {
                    scheduleRetry();
                }
                break;

            case State::BackoffWaiting:
                if (now >= _nextActionAtMs) startConnect();
                break;

            case State::Online:
                if (!_client.isConnected()) {
                    _logger.warn("MQTT", "Disconnected");
                    {
                        auto &gate = iotsmartsys::core::ConnectivityGate::instance();
                        gate.clearBits(iotsmartsys::core::ConnectivityGate::MQTT_CONNECTED);
                    }
                    scheduleRetry();
                } else {
                    // tenta drenar aos poucos (sem travar)
                    drainQueue();
                }
                break;
        }
    }

    // QoS0: se offline -> enfileira (se tiver espaço)
    bool publish(const char* topic, const void* payload, std::size_t len, bool retain=false)
    {
        if (_client.isConnected()) {
            return _client.publish(topic, payload, len, retain);
        }
        return enqueue(topic, payload, len, retain);
    }

    bool subscribe(const char* topic)
    {
        // guarda para resubscribe
        if (_subCount < MaxTopics) {
            _subs[_subCount++] = topic;
        } else {
            _logger.warn("MQTT", "Subscribe list full; topic=%s", topic);
            return false;
        }

        if (_client.isConnected()) {
            return _client.subscribe(topic);
        }
        return true;
    }

    // callback opcional para entregar mensagens à sua camada de roteamento
    void setOnMessage(iotsmartsys::core::MqttOnMessageFn cb, void* user)
    {
        _userMsgCb = cb;
        _userMsgUser = user;
    }

    bool isOnline() const { return _state == State::Online && _client.isConnected(); }

private:
    enum class State : uint8_t { Idle, Connecting, Online, BackoffWaiting };

    struct QueuedMsg {
        const char* topic;     // ponteiro para string estática/literal (sem copiar)
        bool retain;
        uint16_t len;
        uint8_t payload[MaxPayload];
    };

private:
    void startConnect()
    {
        const uint32_t now = _time ? _time->nowMs() : 0;

        // Gate: nunca tenta MQTT sem Wi-Fi + IP (evita spam de connect antes da rede)
        {
            auto &gate = iotsmartsys::core::ConnectivityGate::instance();
            if (!gate.isNetworkReady()) {
                _state = State::BackoffWaiting;
                _nextActionAtMs = now + 500; // reavalia logo, sem bloquear
                return;
            }
        }

        _state = State::Connecting;
        _logger.debug("MQTT", "Starting connection attempt %lu", (unsigned long)(_attempt + 1));
        _nextActionAtMs = now + 15000; // soft-timeout de conexão
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

        for (uint8_t i=0; i<exp; ++i) {
            if (base > (_policy.maxBackoffMs / 2)) { base = _policy.maxBackoffMs; break; }
            base *= 2;
        }
        if (base > _policy.maxBackoffMs) base = _policy.maxBackoffMs;

        uint32_t jitter = 0;
#ifdef ESP32
        if (_policy.jitterMs) jitter = (uint32_t)(esp_random() % (_policy.jitterMs + 1));
#endif
        return base + jitter;
    }

    void resubscribeAll()
    {
        for (std::size_t i=0; i<_subCount; ++i) {
            _client.subscribe(_subs[i]);
        }
    }

    void drainQueue()
    {
        // drena 1 por chamada para não travar o loop
        if (_qCount == 0 || !_client.isConnected()) return;

        QueuedMsg& m = _queue[_qHead];
        _client.publish(m.topic, m.payload, m.len, m.retain);

        _qHead = (_qHead + 1) % QueueLen;
        _qCount--;
    }

    bool enqueue(const char* topic, const void* payload, std::size_t len, bool retain)
    {
        if (len > MaxPayload) {
            _logger.warn("MQTT", "Payload too big (%lu > %lu). Drop.",
                      (unsigned long)len, (unsigned long)MaxPayload);
            return false;
        }
        if (_qCount >= QueueLen) {
            _logger.warn("MQTT", "Queue full. Drop.");
            return false;
        }

        QueuedMsg& m = _queue[_qTail];
        m.topic = topic;
        m.retain = retain;
        m.len = (uint16_t)len;
        for (std::size_t i=0; i<len; ++i) m.payload[i] = ((const uint8_t*)payload)[i];

        _qTail = (_qTail + 1) % QueueLen;
        _qCount++;
        return true;
    }

    static void onMessageThunk(void* user, const iotsmartsys::core::MqttMessageView& msg)
    {
        auto* self = static_cast<MqttService*>(user);
        if (self->_userMsgCb) self->_userMsgCb(self->_userMsgUser, msg);
    }

private:
    iotsmartsys::core::IMqttClient& _client;
    iotsmartsys::core::ILogger& _logger;
    iotsmartsys::core::ITimeProvider* _time;

    iotsmartsys::core::MqttConfig _cfg{};
    RetryPolicy _policy{};

    State _state{State::Idle};
    uint32_t _attempt{0};
    uint32_t _nextActionAtMs{0};

    // subscribe list (sem heap): guarde só tópicos "const char*"
    const char* _subs[MaxTopics]{};
    std::size_t _subCount{0};

    // queue ring-buffer (sem heap)
    QueuedMsg _queue[QueueLen]{};
    std::size_t _qHead{0}, _qTail{0}, _qCount{0};

    // user callback
    iotsmartsys::core::MqttOnMessageFn _userMsgCb{nullptr};
    void* _userMsgUser{nullptr};
};

} // namespace iotsmartsys::app