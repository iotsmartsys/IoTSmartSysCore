#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include "Contracts/Connectivity/ConnectivityGate.h"
#include "Contracts/Settings/SettingsGate.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Providers/Time.h"
#include "Contracts/Transports/ITransportChannel.h"
#include "Contracts/Mqtt/IMqttClient.h"
#include "Contracts/Settings/IReadOnlySettingsProvider.h"

namespace iotsmartsys::app
{

    struct RetryPolicy
    {
        uint32_t initialBackoffMs{1000};
        uint32_t maxBackoffMs{60000};
        uint32_t jitterMs{250};
        uint8_t maxFastRetries{5};
        bool clearQueueOnReconnect{false};
    };

    struct MqttOfflineQueueStats
    {
        uint32_t enqueued{0};
        uint32_t drained{0};
        uint32_t droppedInvalidTopic{0};
        uint32_t droppedTopicTooLong{0};
        uint32_t droppedPayloadTooBig{0};
        uint32_t droppedQueueFull{0};
    };

    enum class MqttDisconnectReason : uint8_t
    {
        None = 0,
        NetworkGateClosed,
        ConnectTimeout,
        ConnectionLost,
        ClientInitFailed
    };

    struct MqttReconnectStats
    {
        uint32_t disconnectsTotal{0};
        uint32_t reconnectsTotal{0};
        uint32_t connectTimeoutsTotal{0};
        uint32_t lastOfflineMs{0};
        uint32_t maxOfflineMs{0};
        uint32_t lastBackoffMs{0};
        uint32_t lastAttemptCount{0};
        MqttDisconnectReason lastDisconnectReason{MqttDisconnectReason::None};
    };

    template <std::size_t MaxTopics, std::size_t QueueLen, std::size_t MaxPayload>
    class MqttService : public iotsmartsys::core::ITransportChannel
    {
    public:
        static constexpr std::size_t MaxTopicLen = 128;

        explicit MqttService(iotsmartsys::core::IMqttClient &client,
                             iotsmartsys::core::ILogger &log,
                             iotsmartsys::core::settings::ISettingsGate &settingsGate,
                             iotsmartsys::core::settings::IReadOnlySettingsProvider &settingsProvider);

        bool begin(const iotsmartsys::core::TransportConfig &cfg) override;
        bool begin(const iotsmartsys::core::TransportConfig &cfg,
                   const RetryPolicy &policy);
        void start() override {}
        void stop() override {}

        void handle() override;

        // QoS0: se offline -> enfileira (se tiver espaço)
        bool publish(const char *topic, const void *payload, std::size_t len, bool retain = false) override;

        bool republish(const iotsmartsys::core::TransportMessageView &msg) override;

        bool subscribe(const char *topic) override;

        // callback opcional para entregar mensagens à sua camada de roteamento
        void setOnMessage(iotsmartsys::core::TransportOnMessageFn cb, void *user) override;
        void setOnConnected(iotsmartsys::core::TransportOnConnectedFn cb, void *user) override;
        void setOnDisconnected(iotsmartsys::core::TransportOnDisconnectedFn cb, void *user) override;

        bool isConnected() const override;
        bool isOnline() const;
        std::size_t queuedCount() const { return _qCount; }
        uint32_t offlineDurationMs() const;
        MqttOfflineQueueStats offlineQueueStats() const { return _queueStats; }
        MqttReconnectStats reconnectStats() const { return _reconnectStats; }
        static const char *disconnectReasonToStr(MqttDisconnectReason reason);
        const char *getName() const override { return _client.getName(); }

    private:
        iotsmartsys::core::settings::ISettingsGate &_settingsGate;
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
            char topic[MaxTopicLen];
            bool retain;
            uint16_t len;
            uint8_t payload[MaxPayload];
        };

    private:
        static const char *stateToStr(State s);
        void markDisconnected(MqttDisconnectReason reason, uint32_t now);
        void markConnected(uint32_t now);
        void clearQueue();

        void startConnect();
        void scheduleRetry();
        uint32_t computeBackoffMs() const;
        void resubscribeAll();
        void drainQueue();
        bool enqueue(const char *topic, const void *payload, std::size_t len, bool retain);

        static void onMessageThunk(void *user, const iotsmartsys::core::TransportMessageView &msg);
        static void onConnectedThunk(void *user, const iotsmartsys::core::TransportConnectedView &info);
        static void onDisconnectedThunk(void *user);
        static void onSettingsReadyThunk(iotsmartsys::core::settings::SettingsReadyLevel level, void *ctx);
        void onSettingsReady(iotsmartsys::core::settings::SettingsReadyLevel level);

    private:
        iotsmartsys::core::settings::IReadOnlySettingsProvider &_settingsProvider;
        iotsmartsys::core::IMqttClient &_client;
        iotsmartsys::core::ILogger &_logger;
        iotsmartsys::core::ITimeProvider *_time;

        iotsmartsys::core::TransportConfig _cfg{};
        // persistent storage for strings referenced by _cfg (avoid dangling pointers)
        std::string _uriStr;
        std::string _usernameStr;
        std::string _passwordStr;
        std::string _clientIdStr;
        RetryPolicy _policy{};

        State _state{State::Idle};
        uint32_t _attempt{0};
        uint32_t _nextActionAtMs{0};
        bool _lastNetworkReady{false};
        uint32_t _lastStatusLogAtMs{0};
        uint32_t _statusLogEveryMs{5000};
        bool _clientInitialized{false};

        
        const char *_publishTopic{nullptr};
        // const char *_subs[MaxTopics]{};
        std::string _subs[MaxTopics];
        std::size_t _subCount{0};

        // queue ring-buffer (sem heap)
        QueuedMsg _queue[QueueLen]{};
        std::size_t _qHead{0}, _qTail{0}, _qCount{0};
        MqttOfflineQueueStats _queueStats{};
        MqttReconnectStats _reconnectStats{};
        uint32_t _offlineSinceMs{0};

        // user callback
        iotsmartsys::core::TransportOnMessageFn _userMsgCb{nullptr};
        void *_userMsgUser{nullptr};
        iotsmartsys::core::TransportOnConnectedFn _userConnectedCb{nullptr};
        void *_userConnectedUser{nullptr};
        iotsmartsys::core::TransportOnDisconnectedFn _userDisconnectedCb{nullptr};
        void *_userDisconnectedUser{nullptr};
    };

} // namespace iotsmartsys::app
