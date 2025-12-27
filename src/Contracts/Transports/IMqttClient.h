#pragma once
#include <cstdint>
#include <cstddef>

namespace iotsmartsys::core
{

    struct MqttMessageView
    {
        const char *topic;
        const char *payload; // pode conter '\0' (use len)
        std::size_t payloadLen;
        bool retain;
    };

    using MqttOnMessageFn = void (*)(void *user, const MqttMessageView &msg);
    using MqttOnConnectedFn = void (*)(void *user);
    using MqttOnDisconnectedFn = void (*)(void *user);

    struct MqttConfig
    {
        const char *uri;
        const char *clientId;
        const char *username;
        const char *password;
        uint16_t keepAliveSec{30};
        bool cleanSession{true};
    };

    class IMqttClient
    {
    public:
        virtual ~IMqttClient() = default;

        virtual bool begin(const MqttConfig &cfg) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;

        virtual bool isConnected() const = 0;

        virtual bool publish(const char *topic,
                             const void *payload,
                             std::size_t len,
                             bool retain) = 0; // QoS 0

        virtual bool subscribe(const char *topic) = 0; // QoS 0

        virtual void setOnMessage(MqttOnMessageFn cb, void *user) = 0;
        virtual void setOnConnected(MqttOnConnectedFn cb, void *user) = 0;
        virtual void setOnDisconnected(MqttOnDisconnectedFn cb, void *user) = 0;

    protected:
    };

} // namespace iotsmartsys::core
