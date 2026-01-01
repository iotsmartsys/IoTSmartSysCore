#pragma once
#include <cstdint>
#include <cstddef>

namespace iotsmartsys::core
{

    enum class TransportKind : uint8_t
    {
        Event = 0,
        Command = 1,
        Ack = 2,
        Raw = 3,
    };

    struct TransportMessageView
    {
        const char *topic;
        const char *payload; // pode conter '\0' (use len)
        std::size_t payloadLen;
        bool retain;

        TransportKind kind{TransportKind::Raw};

        uint32_t id{0};
        uint8_t origin{0};
        uint8_t hops{0};
    };

    struct TransportConnectedView
    {
        const char *clientId;
        const char *broker;
        uint16_t keepAliveSec;
    };

    using TransportOnMessageFn = void (*)(void *user, const TransportMessageView &msg);
    using TransportOnConnectedFn = void (*)(void *user, const TransportConnectedView &info);
    using TransportOnDisconnectedFn = void (*)(void *user);

    struct TransportConfig
    {
        const char *uri;
        const char *clientId;
        const char *username;
        const char *password;
        uint16_t keepAliveSec{30};
        bool cleanSession{true};
    };

    class ITransportChannel
    {
    public:
        virtual ~ITransportChannel() = default;

        virtual bool begin(const TransportConfig &cfg) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;

        virtual bool isConnected() const = 0;

        virtual bool publish(const char *topic,
                             const void *payload,
                             std::size_t len,
                             bool retain) = 0; // QoS 0

        virtual bool subscribe(const char *topic) = 0; // QoS 0

        virtual void setOnMessage(TransportOnMessageFn cb, void *user) = 0;
        virtual void setOnConnected(TransportOnConnectedFn cb, void *user) = 0;
        virtual void setOnDisconnected(TransportOnDisconnectedFn cb, void *user) = 0;

    protected:
    };

} // namespace iotsmartsys::core
